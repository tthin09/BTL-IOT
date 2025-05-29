import time
import cv2
from inference_sdk import InferenceHTTPClient

img_path = "c2.jpg"
CLIENT = InferenceHTTPClient(
    api_url="https://classify.roboflow.com",
    api_key="Ss1kbN4DcQN4GZVczhbk"
)

avg = 0
for i in range(5):
    start = time.time()
    result = CLIENT.infer(img_path, model_id="detect-and-classify-single-label-classification-dhxeb/1")

    delta = time.time() - start
    print(f"Request done in {delta:.2f}s")
    avg += delta
print(f"Average: {avg/5:.2f}s")

image = cv2.imread(img_path)
cv2.imshow("Image", image)
cv2.waitKey(0)
cv2.destroyAllWindows()