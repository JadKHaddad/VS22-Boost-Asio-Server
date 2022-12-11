FROM debian:11-slim as builder

RUN apt-get update
RUN apt-get -y install build-essential g++ python-dev autotools-dev libicu-dev libbz2-dev libboost-all-dev libncurses5-dev libncursesw5-dev make

COPY app /home/app
WORKDIR /home/app
RUN make release

FROM debian:11-slim as runner

RUN useradd -ms /bin/bash app
USER app

COPY --from=builder /home/app/server /home/app/server
COPY --from=builder /home/app/client /home/app/client

WORKDIR /home/app

# docker build -t debian-boost-ncurses-release .
# docker run -it --rm --name debian-boost-ncurses-release debian-boost-ncurses-release
# docker exec -it debian-boost-ncurses-release bash