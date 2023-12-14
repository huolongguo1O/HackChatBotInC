git clone https://github.com/curl/curl.git
cd curl
cmake . -DENABLE_WEBSOCKETS=ON -DBUILD_STATIC_LIBS=ON -DBUILD_SHARED_LIBS=OFF -DHTTP_ONLY=ON
make -j4
make install
cd ..
gcc cws.c cJSON.c handler.c `./curl/curl-config --static-libs` -o cbot
