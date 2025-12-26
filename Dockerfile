FROM gcc:latest
RUN apt-get update && apt-get install -y libpqxx-dev git cmake
WORKDIR /app
COPY . .
# Download Crow headers
RUN git clone https://github.com/CrowCpp/Crow.git && cp -r Crow/include/* /usr/local/include/
# Compile
RUN g++ main.cpp -lpqxx -lpq -o server
# Start the server
CMD ["./server"]