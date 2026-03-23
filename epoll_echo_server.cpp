#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>

using namespace std;

const int PORT = 8080;
const int MAX_EVENTS = 10;
const int BUFFER_SIZE = 1024;

bool setNonBlocking(int fd) {
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1) {
		perror("fcntl F_GETFL");
		return false;
	}

	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
		perror("fcntl F_SETFL");
		return false;
	}

	return true;
	
}

int main(void) {
    // 1. Create listening socket
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        perror("Socket");
        return 1;
    }

	// Set listenfd to non-blocking
	if (!setNonBlocking(listenfd)) {
		close(listenfd);
		return 1;
	}

    // 2. Bind IP and Port
    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if( bind(listenfd, (sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("bind");
        close(listenfd);
        return 1;
    }

    // 3. Start listening
    if (listen(listenfd, MAX_EVENTS) == -1) {
        perror("Listen");
        close(listenfd);
        return 1;
    }

    cout << "Server is listening on port " << PORT << endl;

    // 4. Create epoll instance
    int epfd = epoll_create(1);
    if (epfd == -1) {
        perror("epoll_create");
        close(listenfd);
        return 1;
    }

    // 5. Add listenfd into epoll
    epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = listenfd;

    if ( epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev) == -1 ) {
        perror("epoll_ctl: listen");
        close(listenfd);
        close(epfd);
        return 1;
    }

    // 6. Event array used by epoll_wait
    epoll_event events[MAX_EVENTS];

    while (true) {
        // 7. Wait for events
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("epoll_wait");
            break;
        }

        // 8. Handle ready events
        for (int i = 0; i < nfds; i++) {
            int currentFd = events[i].data.fd;

            // 8.1 New client connection
            if (currentFd == listenfd) {

               while (true) {
				sockaddr_in clientAddr;                                           	
                socklen_t clientLen = sizeof(clientAddr);
			                                                                      
                int connfd = accept(listenfd, (sockaddr*)&clientAddr, &clientLen);
                if (connfd == -1) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) { 
						break;
					} else {
     					perror("accept");
	     			    break;
                    }
				}
                                                                                   
                if (!setNonBlocking(connfd)) {
                	close(connfd);
                	continue;
                }
                                                                                   
                cout << "New client connected, fd = " << connfd << endl;
				                                                                   
				epoll_event clientEv;
				clientEv.events = EPOLLIN | EPOLLET;
				clientEv.data.fd = connfd;
                                                                                   
                if (epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &clientEv) == -1) {
                    perror("epoll_ctl: connfd");
                    close(connfd);
                    continue;
                }

		      }

            }
            // 8.2 Existing client sent data
            else {
            	bool shouldClose = false;

				while (true) {
					char buffer[BUFFER_SIZE];
                    memset(buffer, 0, sizeof(buffer));
				                                                         
                    int bytesRead = read(currentFd, buffer, sizeof(buffer));
				
					if (bytesRead > 0) {
						cout << "Received from fd: " << currentFd << ":";
						cout.write(buffer, bytesRead);
						cout << endl;
					
						int totalWritten = 0;                                                                    	
					    while (totalWritten < bytesRead) {
					    	int bytesWritten = write(currentFd, buffer + totalWritten, bytesRead - totalWritten);
					                                                                                              
                        	if (bytesWritten > 0) {
					    		totalWritten += bytesWritten;
					    	} else if ( bytesWritten == -1 && (errno == EAGAIN || errno == EWOULDBLOCK) ) {
					    		// For now, just stop writing and try next in a more advanced version;
					            break;
		                	} else {
					    		perror("write");
					    		shouldClose = true;
					    		break;
					        }
						}

						if (shouldClose) {
							break;
						}
					
					} else if (bytesRead == 0) {
                    	cout << "Client disconnected, fd = " << currentFd << endl;
						shouldClose = true;
						break;
					} else {
						if (errno == EAGAIN || errno == EWOULDBLOCK) {
							// all currently avilable data has been read
							break;
						} else {
							perror("read");
							shouldClose = true;
							break;
						}

					}
				}

				if (shouldClose) {
					epoll_ctl(epfd, EPOLL_CTL_DEL, currentFd, nullptr);
					close(currentFd);
				}


            }
        }
    }

    close(listenfd);
    close(epfd);
    return 0;
}
