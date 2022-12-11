# VS 22

```sh
docker build -t debian-boost-ncurses-release .
```

```sh
docker run -it --rm --name debian-boost-ncurses-release --entrypoint /home/app/server debian-boost-ncurses-release 80
```

```sh
docker exec -it debian-boost-ncurses-release /home/app/client 127.0.0.1 80
```
