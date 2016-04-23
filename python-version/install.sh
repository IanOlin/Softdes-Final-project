#/bin/bash

repository = "https://github.com/Valodim/python-pulseaudio.git"
git clone "$repository"
cd python-pulseaudio
python setup.py install
