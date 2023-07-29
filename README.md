# demo-realtime-networking

1.Create new network in docker
  docker network create my_network
  
2.cd to server and client folder and build image
  - docker build -t my_server -f Dockerfile.server .
  - docker build -t my_client -f Dockerfile.client .
    
3.Run 2 containers client and server using network created
  - docker run -d --name server_container --network my_network my_server
  - docker run -it --name client_container --network my_network my_client
