# git clone https://github.com/curl/curl.git
# You can down load by submodule
cd curl
cmake . -DENABLE_WEBSOCKETS=OFF -DBUILD_STATIC_LIBS=ON -DBUILD_SHARED_LIBS=OFF -DHTTP_ONLY=ON
make -j4
make install
cd ..
gcc cws.c cJSON.c handler.c `./curl/curl-config --static-libs` -o cbot
