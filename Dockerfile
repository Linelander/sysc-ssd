FROM debian:trixie

RUN apt-get update && apt-get install -y \
build-essential cmake libsystemc-dev man-db \
&& rm -rf /var/lib/apt/lists/*

RUN useradd sysc
USER sysc

WORKDIR /work