from fastapi import FastAPI, UploadFile, File
from fastapi.responses import JSONResponse
import shutil, os
from compare_mp4 import compare_two_videos   # ✅ 유사도 분석 코드 불러오기 

app = FastAPI(title="Pose Similarity API")

# 결과 저장
last_result = {}

@app.post("/status")
async def upload(file1: UploadFile = File(...), file2: UploadFile = File(...)):
    try:
        # 파일 저장
        path1, path2 = f"temp_{file1.filename}", f"temp_{file2.filename}"
        with open(path1, "wb") as buffer:
            shutil.copyfileobj(file1.file, buffer)
        with open(path2, "wb") as buffer:
            shutil.copyfileobj(file2.file, buffer)

        # 유사도 분석
        result = compare_two_videos(path1, path2, use_flip=True, sync=True)

        # 결과 저장
        global last_result
        last_result = result

        # 파일 삭제
        os.remove(path1)
        os.remove(path2)

        return JSONResponse(content={"format": last_result[0], "total_score": last_result[1]})
    except Exception as e:
        return JSONResponse(content={"error": str(e)}, status_code=500)


@app.get("/result")
async def result():
    if not last_result:
        return JSONResponse(
            content={"message": "아직 결과가 없습니다. 먼저 /upload 호출하세요."},
            status_code=404
        )
    return JSONResponse(content={"format": last_result[0], "total_score": last_result[1]})
