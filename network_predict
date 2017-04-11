from PIL import Image
from keras.models import Sequential
from keras.layers import Convolution2D, MaxPooling2D
from keras.layers import Activation, Dropout, Flatten, Dense

from keras import backend as K
K.set_image_dim_ordering('tf')

import numpy as np
import os

COLOR_MODE = 'grayscale'
TRY_PATH = os.path.normpath('data/img_try')
WEIGHTS_PATH = os.path.normpath('data/weights/gender_grayscale50.hdf5')
IMAGE_PATH = os.path.normpath('data/img_try/19.jpg')
IMG_WIDTH, IMG_HEIGHT = 139, 139

img = Image.open(IMAGE_PATH)
img = img.resize((IMG_WIDTH, IMG_HEIGHT))
img = img.convert('L' if COLOR_MODE == 'grayscale' else 'RGB')
img_try = np.asarray(img, dtype='float32')
img_try = img_try.reshape((IMG_WIDTH, IMG_HEIGHT, 1 if COLOR_MODE == 'grayscale' else 3))
img_try = np.expand_dims(img_try, axis=0)

model = Sequential()

model.add(Convolution2D(32, 3, 3, input_shape=(IMG_WIDTH, IMG_HEIGHT, 1 if COLOR_MODE == 'grayscale' else 3)))
model.add(Activation('relu'))
model.add(MaxPooling2D())

model.add(Convolution2D(32, 3, 3))
model.add(Activation('relu'))
model.add(MaxPooling2D())

model.add(Convolution2D(64, 3, 3))
model.add(Activation('relu'))
model.add(MaxPooling2D())

model.add(Flatten())
model.add(Dense(64))
model.add(Activation('relu'))
model.add(Dropout(0.5))
model.add(Dense(1))
model.add(Activation('sigmoid'))

model.compile(
    loss = 'binary_crossentropy',
    optimizer = 'rmsprop',
    metrics = ['accuracy'])

model.load_weights(WEIGHTS_PATH)

prediction = model.predict(img_try, verbose=1)
print(prediction)
print('woman' if prediction == [[1.0]] else 'man')
