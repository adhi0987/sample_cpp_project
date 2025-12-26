FROM gcc:latest
RUN apt-get update && apt-get install -y libpqxx-dev git cmake
WORKDIR /app
COPY . .
# We download Crow header-only library directly
RUN git clone https://github.com/CrowCpp/Crow.git && cp -r Crow/include/* /usr/local/include/
RUN g++ main.cpp -lpqxx -lpq -o server
CMD ["./server"]