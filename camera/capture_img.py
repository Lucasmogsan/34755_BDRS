#!/usr/bin/env python

"""
This script opens the specified video capture device for a video stream. 
Images captured will be saved in the specified base folder.
Press c to capture an image. Press q to quit.
Images captured can be used for camera calibration.
"""

import cv2
import os


def save_frame_camera_key(device_num, dir_path, basename,
                          ext='jpg', delay=1, window_name='frame'):
    cap = cv2.VideoCapture(device_num)

    if not cap.isOpened():
        print("Can't receive frame. Exiting ...")
        return

    os.makedirs(dir_path, exist_ok=True)
    base_path = os.path.join(dir_path, basename)

    n = 0
    while True:
        ret, frame = cap.read()
        cv2.imshow(window_name, frame)
        key = cv2.waitKey(delay) & 0xFF
        if key == ord('c'):
            img_name = '{}_{}.{}'.format(base_path, n, ext)
            cv2.imwrite(img_name, frame)
            n += 1
            print("Images captured: ", n)
        elif key == ord('q'):
            break

    print("Stopping video stream")
    cv2.destroyWindow(window_name)
    cap.release()


save_frame_camera_key(1, 'calib_imgs', 'cam_capture')
