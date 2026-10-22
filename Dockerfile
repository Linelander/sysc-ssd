FROM debian:trixie
WORKDIR /work

RUN apt-get update && apt-get install -y \
	build-essential cmake libsystemc-dev \
&& rm -rf /var/lib/apt/lists/*

COPY src ./src
EXPOSE 8080

RUN useradd sysc
USER sysc