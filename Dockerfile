FROM gcc:latest

RUN apt-get update && apt-get install -y \
    libpqxx-dev \
    libasio-dev \
    git \
    cmake

WORKDIR /app

# Fix: Combined into a valid COPY command
COPY . .

# Download Crow headers
RUN git clone https://github.com/CrowCpp/Crow.git && cp -r Crow/include/* /usr/local/include/

# Fix: Added -pthread and -std=c++17 flags for a successful build
RUN g++ -std=c++17 main.cpp -lpqxx -lpq -pthread -o server

CMD ["./server"]