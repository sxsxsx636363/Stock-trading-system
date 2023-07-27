FROM ubuntu:latest
RUN mkdir /temp
ADD . /temp
WORKDIR /temp
ENV DEBIAN_FRONTEND=noninteractive
ENV CPLUS_INCLUDE_PATH=/usr/local/include/boost/asio:/usr/local/include/boost
RUN pwd
RUN echo "*******"
RUN ls -a
RUN apt-get update && apt-get -y install g++ make libboost-all-dev libpqxx-dev postgresql 

RUN apt-get update && apt-get install -y \
    wget \
    build-essential \
    && rm -rf /var/lib/apt/lists/*
RUN wget https://boostorg.jfrog.io/artifactory/main/release/1.81.0/source/boost_1_81_0.tar.gz \
    && tar -xzvf boost_1_81_0.tar.gz 
RUN pwd
RUN echo "*******"
RUN ls -a
RUN cd boost_1_81_0 \
    && ./bootstrap.sh --prefix=/usr/local \
    && ./b2 install 

RUN pwd
RUN echo "*******"
RUN ls -a
RUN cd boost_1_81_0 \
    ls -a\
    pwd\
    cp -r boost /usr/local/include \
    && cp -r libs /usr/local/lib
