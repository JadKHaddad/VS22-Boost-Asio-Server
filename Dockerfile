FROM debian:11-slim

RUN apt-get update
RUN apt-get -y install build-essential g++ python-dev autotools-dev libicu-dev libbz2-dev libboost-all-dev libncurses5-dev libncursesw5-dev make

WORKDIR /home/app

CMD ["bash"]

# docker build -t debian-boost-ncurses .
# docker run -it --rm -v ${PWD}/app:/home/app/ --name debian-boost-ncurses debian-boost-ncurses
# docker exec -it debian-boost-ncurses bash