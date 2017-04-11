from keras.preprocessing.image import ImageDataGenerator
from keras.models import Sequential
from keras.layers import Convolution2D, MaxPooling2D
from keras.layers import Activation, Dropout, Flatten, Dense

from keras import backend as K
K.set_image_dim_ordering('tf')

import os

IMG_WIDTH, IMG_HEIGHT = 150, 150
TRAIN_DATA_DIR = os.path.normpath('data/img_train')
VALID_DATA_DIR = os.path.normpath('data/img_valid')
WEIGHTS_DIR = 'data/weights/'
COLOR_MODE = 'grayscale'
NB_TRAIN_SAMPLES = 21615
NB_VALIDATION_SAMPLES = 1993
NB_EPOCH = 50

train_datagen = ImageDataGenerator(
    rescale=1./255,
    zoom_range=0.2,
    shear_range=0.2,
    rotation_range=10,
    horizontal_flip=True)

validation_datagen = ImageDataGenerator(rescale=1. / 255)

train_generator = train_datagen.flow_from_directory(
    TRAIN_DATA_DIR,
    target_size=(IMG_WIDTH, IMG_HEIGHT),
    color_mode=COLOR_MODE,
    class_mode='binary')

validation_generator = validation_datagen.flow_from_directory(
    VALID_DATA_DIR,
    target_size=(IMG_WIDTH, IMG_HEIGHT),
    batch_size=32,
    color_mode=COLOR_MODE,
    class_mode='binary')

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
    loss='binary_crossentropy',
    optimizer='rmsprop',
    metrics=['accuracy'])

model.fit_generator(
    train_generator,
    samples_per_epoch=NB_TRAIN_SAMPLES,
    nb_epoch=NB_EPOCH,
    validation_data=validation_generator,
    nb_val_samples=NB_VALIDATION_SAMPLES)

save_as = 'try.hdf5'

model.save_weights(os.path.normpath(WEIGHTS_DIR + save_as))
