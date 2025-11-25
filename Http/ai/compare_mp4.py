import cv2
import numpy as np
import mediapipe as mp
from yt_dlp import YoutubeDL
from typing import Dict, List, Optional, Tuple
import os

# ---------------------------
# 좌우 flip 모드
# ---------------------------
flip_mode = True  # 좌우 flip 모드 활성화 여부

# 신체 부위 인덱스
limbs = {
    "face": [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12],
    "left_arm": [11, 13, 15, 17, 19, 21],
    "right_arm": [12, 14, 16, 18, 20, 22],
    "left_leg": [23, 25, 27, 29, 31],
    "right_leg": [24, 26, 28, 30, 32],
    "body": [11, 12, 23, 24]
}


def download_youtube(url: str, filename: str = "input.mp4") -> str:
    ydl_opts = {"format": "best", "outtmpl": filename, "quiet": True}
    with YoutubeDL(ydl_opts) as ydl:
        ydl.download([url])
    return filename


def load_reference_pose_keypoints(np_path: str) -> List[Optional[np.ndarray]]:
    raw_data = np.load(np_path, allow_pickle=True)
    reference: List[Optional[np.ndarray]] = []
    for frame in raw_data:
        if isinstance(frame, dict) and "pose" in frame:
            ref_pose = np.array(frame["pose"], dtype=np.float32)
            reference.append(ref_pose)
        else:
            reference.append(None)
    return reference


def compare_video(
    video_path: str,
    reference_np_path: str = "holistic_kps.npy",
    use_flip: bool = True,
    max_frames: Optional[int] = None,
) -> Dict:
    """Compare a video file against the reference keypoints file.

    Returns a dict with average score and per-limb average scores.
    """
    mp_pose = mp.solutions.pose
    pose = mp_pose.Pose(static_image_mode=False, min_detection_confidence=0.5)

    reference_kps = load_reference_pose_keypoints(reference_np_path)

    cap = cv2.VideoCapture(video_path)
    frame_idx = 0
    frame_scores: List[float] = []
    limb_accumulators: Dict[str, List[float]] = {name: [] for name in limbs.keys()}

    while cap.isOpened():
        if max_frames is not None and frame_idx >= max_frames:
            break

        ret, frame = cap.read()
        if not ret:
            break

        try:
            rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
            results = pose.process(rgb_frame)

            if not results.pose_landmarks:
                frame_idx += 1
                continue

            if frame_idx >= len(reference_kps):
                frame_idx += 1
                continue

            kps = np.array([[lm.x, lm.y, lm.z] for lm in results.pose_landmarks.landmark], dtype=np.float32)
            ref_kps = reference_kps[frame_idx]
            if ref_kps is None:
                frame_idx += 1
                continue

            if kps.shape[0] < 33 or ref_kps.shape[0] < 33:
                frame_idx += 1
                continue

            torso_center = (kps[23] + kps[24]) / 2.0
            norm_kps = kps - torso_center

            ref_center = (ref_kps[23] + ref_kps[24]) / 2.0
            ref_kps_norm = ref_kps - ref_center

            if use_flip:
                ref_kps_norm[:, 0] *= -1

            diff = norm_kps - ref_kps_norm
            distances = np.linalg.norm(diff[:, :2], axis=1)
            score = 1.0 - distances.mean()
            frame_scores.append(float(score))

            for limb_name, indices in limbs.items():
                limb_kps = norm_kps[indices]
                ref_limb_kps = ref_kps_norm[indices]
                d = limb_kps - ref_limb_kps
                distances = np.linalg.norm(d[:, :2], axis=1)
                limb_score = 1.0 - distances.mean()
                limb_accumulators[limb_name].append(float(limb_score))

        except Exception as e:
            print(f"⚠️ Frame {frame_idx} 처리 중 예외 발생: {e}")

        frame_idx += 1

    cap.release()

    avg_score = float(np.mean(frame_scores)) if frame_scores else 0.0
    limb_avgs = {name: (float(np.mean(vals)) if vals else 0.0) for name, vals in limb_accumulators.items()}

    return {
        "average_score": avg_score,
        "frames_evaluated": len(frame_scores),
        "per_limb_average": limb_avgs,
    }


def _extract_pose_sequence_from_video(video_path: str, max_frames: Optional[int], use_flip: bool) -> Tuple[List[np.ndarray], float]:
    """Helper: extract normalized pose keypoints for each frame from a video.

    Returns a list of keypoint arrays (or None) aligned by frame index.
    """
    mp_pose = mp.solutions.pose
    pose = mp_pose.Pose(static_image_mode=False, min_detection_confidence=0.5)

    cap = cv2.VideoCapture(video_path)
    frame_idx = 0
    seq: List[np.ndarray] = []

    while cap.isOpened():
        if max_frames is not None and frame_idx >= max_frames:
            break
        ret, frame = cap.read()
        if not ret:
            break

        try:
            rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
            results = pose.process(rgb_frame)
            if not results.pose_landmarks:
                seq.append(None)
                frame_idx += 1
                continue

            kps = np.array([[lm.x, lm.y, lm.z] for lm in results.pose_landmarks.landmark], dtype=np.float32)
            if kps.shape[0] < 33:
                seq.append(None)
                frame_idx += 1
                continue

            torso_center = (kps[23] + kps[24]) / 2.0
            norm_kps = kps - torso_center
            seq.append(norm_kps)

        except Exception as e:
            print(f"⚠️ extract frame {frame_idx} exception: {e}")
            seq.append(None)

        frame_idx += 1

    cap.release()
    # get fps (fallback to 30 if unknown)
    fps = cap.get(cv2.CAP_PROP_FPS)
    try:
        fps = float(fps)
        if fps <= 0 or np.isnan(fps):
            fps = 30.0
    except Exception:
        fps = 30.0
    return seq, fps


def compare_two_videos(
    video_a: str,
    video_b: str,
    max_frames: Optional[int] = None,
    use_flip: bool = True,
    sync: bool = False,
    max_offset: int = 30,
    chunk_seconds: int = 5,
) -> Dict:
    """Compare two videos by extracting pose sequences from both and computing frame-by-frame similarity.

    Returns a dict with overall average score, frames compared, and per-limb averages.
    """
    seq_a, fps_a = _extract_pose_sequence_from_video(video_a, max_frames=max_frames, use_flip=False)
    seq_b, fps_b = _extract_pose_sequence_from_video(video_b, max_frames=max_frames, use_flip=False)

    # Treat video_b as the reference. If use_flip is requested, flip x for reference frames
    if use_flip:
        for idx, frame_kps in enumerate(seq_b):
            if frame_kps is None:
                continue
            seq_b[idx] = frame_kps.copy()
            seq_b[idx][:, 0] *= -1

    len_a = len(seq_a)
    len_b = len(seq_b)

    # If sync requested, search best integer frame offset within [-max_offset, max_offset]
    best_offset = 0
    if sync:
        best_score = -999.0
        # precompute flattened 2D-only feature arrays with None handling
        for offset in range(-max_offset, max_offset + 1):
            scores = []
            # i indexes frames in A, j in B
            for i in range(len_a):
                j = i + offset
                if j < 0 or j >= len_b:
                    continue
                a = seq_a[i]
                b = seq_b[j]
                if a is None or b is None:
                    continue
                diff = a - b
                distances = np.linalg.norm(diff[:, :2], axis=1)
                scores.append(1.0 - distances.mean())
            if not scores:
                continue
            avg = float(np.mean(scores))
            if avg > best_score:
                best_score = avg
                best_offset = offset

    # Now compute final comparison using chosen offset (best_offset if sync else 0)
    offset = best_offset if sync else 0

    frame_scores: List[float] = []
    limb_accumulators: Dict[str, List[float]] = {name: [] for name in limbs.keys()}

    for i in range(len_a):
        j = i + offset
        if j < 0 or j >= len_b:
            continue
        a = seq_a[i]
        b = seq_b[j]
        if a is None or b is None:
            continue

        diff = a - b
        distances = np.linalg.norm(diff[:, :2], axis=1)
        score = 1.0 - distances.mean()
        frame_scores.append(float(score))

        for limb_name, indices in limbs.items():
            limb_a = a[indices]
            limb_b = b[indices]
            d = limb_a - limb_b
            distances = np.linalg.norm(d[:, :2], axis=1)
            limb_score = 1.0 - distances.mean()
            limb_accumulators[limb_name].append(float(limb_score))

    avg_score = float(np.mean(frame_scores)) if frame_scores else 0.0
    limb_avgs = {name: (float(np.mean(vals)) if vals else 0.0) for name, vals in limb_accumulators.items()}

    # Chunking: use fps_a to compute frames per chunk
    fps = fps_a if fps_a and fps_a > 0 else 30.0
    frames_per_chunk = max(1, int(round(fps * chunk_seconds)))

    # Prepare ordered per-limb lists (left_arm, right_arm, left_leg, right_leg)
    limb_order = ["left_arm", "right_arm", "left_leg", "right_leg"]
    per_limb_lists = [limb_accumulators[name] for name in limb_order]

    # number of chunks
    num_frames = len(per_limb_lists[0]) if per_limb_lists and per_limb_lists[0] else len(frame_scores)
    if num_frames == 0:
        chunked: List[List[float]] = []
    else:
        num_chunks = int(np.ceil(num_frames / frames_per_chunk))
        chunked = []
        for c in range(num_chunks):
            start = c * frames_per_chunk
            end = start + frames_per_chunk
            chunk_vals = []
            for lst in per_limb_lists:
                seg = lst[start:end]
                val = float(np.mean(seg)) if seg else 0.0
                chunk_vals.append(val)
            chunked.append(chunk_vals)

    # Return as [chunked_per_limb, total_score] to match requested output format
    return [chunked, avg_score]


if __name__ == "__main__":
    # legacy script usage: keep simple behavior
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument("--video", required=False, help="Path to local mp4 file")
    parser.add_argument("--youtube", required=False, help="YouTube URL to download and evaluate")
    parser.add_argument("--reference", default="holistic_kps.npy")
    args = parser.parse_args()

    if args.youtube:
        video_path = download_youtube(args.youtube, filename="input.mp4")
    elif args.video:
        video_path = args.video
    else:
        raise SystemExit("Provide --video or --youtube to run")

    result = compare_video(video_path, reference_np_path=args.reference, use_flip=flip_mode)
    print("✅ Processing 완료", f"평균 score: {result['average_score']:.4f}")