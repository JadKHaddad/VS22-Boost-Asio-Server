FROM debian:11-slim

RUN apt-get update
RUN apt-get -y install build-essential g++ python-dev autotools-dev libicu-dev libbz2-dev libboost-all-dev
RUN apt-get -y install make

WORKDIR /home/app

CMD ["bash"]

# docker build -t debian-boost .
# docker run -it --rm -v ${PWD}:/home/app/ debian-boost