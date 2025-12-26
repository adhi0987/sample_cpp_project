FROM gcc:latest

# Added libasio-dev to the install list
RUN apt-get update && apt-get install -y \
    libpqxx-dev \
    libasio-dev \
    git \
    cmake

WORKDIR /app

COPY . .

# Download Crow headers
RUN git clone https://github.com/CrowCpp/Crow.git && cp -r Crow/include/* /usr/local/include/

# Compile (Note: Asio is header-only, so no extra -l flag is needed for it)
RUN g++ main.cpp -lpqxx -lpq -o server

# Start the server
CMD ["./server"]