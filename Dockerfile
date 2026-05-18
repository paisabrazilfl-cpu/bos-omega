FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    libcurl4-openssl-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY bos_omega_production.c .

RUN gcc bos_omega_production.c -o bos_omega_prod -lcurl

CMD ["./bos_omega_prod"]
