FROM debian:trixie


RUN apt-get update && apt-get install -y \
build-essential cmake libsystemc-dev man-db \
&& rm -rf /var/lib/apt/lists/*


RUN python3 -m venv /opt/venv
ENV PATH="/opt/venv/bin:$PATH"


COPY requirements.txt /tmp/requirements.txt
RUN pip install --no-cache-dir -r /tmp/requirements.txt 


RUN useradd sysc
USER sysc


WORKDIR /work