#include "includes/Server.h"
#define BUFFER_SIZE 1024

void Server::start(const std::string& host, int port) {
	m_server_sock = create_server_socket_bind_listen(host, port);
	if (!ISVALIDSOCKET(m_server_sock)) return;

	//std::unique_ptr<ThreadPool> pool(new_task_queue());
	fd_set master_set, read_set;
	FD_ZERO(&master_set);
	FD_SET(m_server_sock, &master_set);
	SOCKET max_socket = m_server_sock;
	struct timeval timeout;
	timeout.tv_sec = static_cast<long>(0);
	timeout.tv_usec = static_cast<decltype(timeout.tv_usec)>(10000);

	UserDatabase& users_database = UserDatabase::get_instance();
	auto users = users_database.get_users();
	//User::connection_info connection_info;
	//std::unique_ptr<ThreadPool> threadpool(new_task_queue());
	while (m_server_sock != INVALID_SOCKET) {
		//int socket_count = select_read(m_server_sock);
		// when a new connection is accepted (::accept the sd is added to master set. here read_Set copies it
		// and gets the latest copy of master
		read_set = master_set; // Copy the master set to the read set
		int socket_count = select(max_socket + 1, &read_set, nullptr, nullptr, &timeout);
		if (socket_count < 0) {
			std::cerr << "select failed: " << GETSOCKETERRNO() << std::endl;
			break;
		}
		
		for (int i = 0; i <= max_socket; i++) {
			//std::cout << "inside loop\n";
			if (FD_ISSET(i, &read_set)) {
				std::cout << "inside if fd_isset\n";
				if (i == m_server_sock) {
					std::cout << "before handle new\n";
					handle_new_connection(m_server_sock, master_set, max_socket, users);
				}
				else {
					std::cout << "before handle client\n";
					/*threadpool->enqueue(std::bind(&Server::handle_client_message,this , i, std::ref(master_set), std::ref(m_users)));*/
					std::thread thr(&Server::handle_client_message,this, i, std::ref(master_set), std::ref(users));
					thr.join();
					//handle_client_message(i, master_set, m_users);
				}
			}
		}
	}
}

void Server::handle_new_connection(const SOCKET server_socket, fd_set& master_set, SOCKET& max_socket, std::unordered_map<SOCKET, User>& m_users) {
	std::cout << "entered new conn" << std::endl;
	struct sockaddr_storage client_address;
	socklen_t client_len = sizeof(client_address);
	SOCKET client_sock = accept(m_server_sock, (struct sockaddr*)&client_address, &client_len);

	/*UserDatabase& users_database = UserDatabase::get_instance();
	auto users = users_database.get_users();*/

	m_users[client_sock] = User();
	m_users[client_sock].conn_info.address = client_address;
	m_users[client_sock].conn_info.address_length = client_len;

	if (!ISVALIDSOCKET(client_sock)) {
		std::cerr << "accept failed: " << GETSOCKETERRNO() << std::endl;
		return;
	}

	std::cout << "Client connected." << std::endl;
	FD_SET(client_sock, &master_set);
	if (client_sock > max_socket)
		max_socket = client_sock;

	// Send registration or login prompt to the new client
	std::string prompt = "Welcome! Please register or login:\n";
	send(client_sock, prompt.c_str(), prompt.size(), 0);
}

void Server::send_error(SOCKET client_socket, std::string& error) {
	send(client_socket, error.c_str(), error.size(), 0);
}

void Server::handle_client_message(SOCKET client_socket,
	fd_set& master_Set, 
	std::unordered_map<SOCKET, User>& m_users) {
	std::lock_guard<std::mutex> lock(m_mtx);
	char buffer[BUFFER_SIZE];
	memset(buffer, 0, BUFFER_SIZE);

	int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
	if (bytes_received <= 0) {
		// Connection closed or error
		//std::lock_guard<std::mutex> lock(fd_set_mutex);
		FD_CLR(client_socket, &master_Set);
		CLOSESOCKET(client_socket);
		m_users.erase(client_socket);
		return;
	}
	// receive message from client and trim it
	std::string message(buffer, bytes_received);
	utils::trim(message);
	std::cout << "received " << bytes_received << " bytes " << message << std::endl;

	// parse the message and find the command register or login 
	std::istringstream iss(message);
	std::string command, username, password, email;
	iss >> command;
	auto it = m_users.find(client_socket);
	if (
		//it == m_users.end() || 
		(command == "register" || command == "login")) {
		if (command == "register") {
			// Registration logic
			if (!(iss >> username >> password >> email)) {
				std::string invalid_command = "Invalid command format. Use: register <username> <password> <email> or login <username> <password>\n";
				send_error(client_socket, invalid_command);
				return;
			}

			m_users[client_socket].set_username(username);
			m_users[client_socket].set_password(password);
			m_users[client_socket].set_email(email);
				//= User(username, password, email, client_socket);
			m_users[client_socket].set_is_registered(true);
			//std::string client_address = get_client_address(m_users[client_socket]);
			
			User user = m_users[client_socket];
			getpeername(client_socket, (sockaddr*) &(user.get_address()),
				&(user.get_address_length()));
			char buffer[INET6_ADDRSTRLEN];
			void* addr;
			if (user.get_address().ss_family == AF_INET) {
				// IPv4
				struct sockaddr_in* s = (struct sockaddr_in*)&user.get_address();
				addr = &(s->sin_addr);
			}
			else { // AF_INET6
				// IPv6
				struct sockaddr_in6* s = (struct sockaddr_in6*)&user.get_address();
				addr = &(s->sin6_addr);
			}
			inet_ntop(user.get_address().ss_family, addr, buffer, sizeof(buffer));
			std::string ip_address(buffer);
			std::cout << ip_address << std::endl;

			std::string response = "Registered successfully. Please login.\n";
			send(client_socket, response.c_str(), response.size(), 0);
		}
		else if (command == "login") {
			// Login logic
			// parse the client's message, ensure it is in the correct format
			if (!(iss >> username >> password)) {
				std::string invalid_command = "Invalid command format. Use: login <username> <password>\n";
				send_error(client_socket, invalid_command);
				return;
			}
			// check user database and find the user based on login credentials
			auto it = std::find_if(m_users.begin(), m_users.end(), [&](const auto& pair) {
				return pair.second.get_username() == username && pair.second.get_password() == password;
				});

			// if found in db welcome him
			if (it != m_users.end() && it->second.get_is_registered()) {
				m_users[client_socket].set_is_logged_in(true);
				std::string response = "Login successful.\n";
				send(client_socket, response.c_str(), response.size(), 0);
				std::string welcome_message = "User " + username + " has joined the chat\n";
				broadcast_message(welcome_message, client_socket, m_users);
			}
			else {
				std::string response = "Invalid username or password.\n";
				send(client_socket, response.c_str(), response.size(), 0);
				return;
			}
		}
		else {
			std::string invalid_command = "Invalid command format. Use: register <username> <password> <email> or login <username> <password>\n";
			send_error(client_socket, invalid_command);
			return;
		}
	}
	else if (m_users.find(client_socket)->second.get_is_logged_in() && m_users.find(client_socket)->second.get_is_registered() && m_users.find(client_socket) != m_users.end()) {
		// Handle messages from authenticated m_users
		User user = m_users[client_socket];
		if (user.get_is_logged_in()) {
			// drop client
			if (message == "exit") {
				std::string broadcast = "User " + it->second.get_username() + "has left the chat\n";
				broadcast_message(broadcast, client_socket, m_users);
				//std::lock_guard<std::mutex> lock(fd_set_mutex);
				FD_CLR(client_socket, &master_Set);
				CLOSESOCKET(client_socket);
				m_users.erase(client_socket);
				return;
			}
			// Broadcast message to all other connected clients
			std::string full_message = user.get_username() + ": " + message + '\n';
			broadcast_message(full_message, client_socket, m_users);
		}
		else {
			std::string response = "You must be logged in to send messages.\n";
			send_error(client_socket, response);
			return;
		}
	}
	else {
		std::string error_message = "You need to be logged in to send messages. Please use the following format in order to login: <login> <username> <password>\n";
		send_error(client_socket, error_message);
		return;
	}
}


void Server::broadcast_message(std::string message, SOCKET sender_socket, std::unordered_map<SOCKET, User>& m_users) {
	for (const auto& pair : m_users) {
		if (pair.first != sender_socket && pair.second.get_is_logged_in()) {
			send(pair.first, message.c_str(), message.size(), 0);
		}
	}
}

std::string Server::get_client_address(User& user) {
	static char address_buffer[100];
	int result = getnameinfo((struct sockaddr*)&user.get_address(),
		user.get_address_length(),
		address_buffer, sizeof(address_buffer), 0, 0,
		NI_NUMERICHOST);

	if (result != 0) {
		std::cerr << "getnameinfo() failed: " << gai_strerror(result) << std::endl;
		return "";
	}
	return std::string(address_buffer);
}

int Server::create_server_socket_bind_listen(const std::string& host, int port) {
#if defined(_WIN32)
	WSADATA d;
	if (WSAStartup(MAKEWORD(2, 2), &d)) {
		fprintf(stderr, "Failed to initialize.\n");
		exit(1);
	}
#endif

	printf("Configuring local address...\n");
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	struct addrinfo* bind_address;
	getaddrinfo(0, std::to_string(port).c_str(), &hints, &bind_address);


	printf("Creating socket...\n");
	SOCKET socket_listen;
	socket_listen = socket(bind_address->ai_family,
		bind_address->ai_socktype, bind_address->ai_protocol);
	if (!ISVALIDSOCKET(socket_listen)) {
		std::cerr << "socket() failed. (" << GETSOCKETERRNO() << ")\n";
		return -1;
	}

	int option = 0;
	if (setsockopt(socket_listen, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&option, sizeof(option))) {
		fprintf(stderr, "setsockopt() failed. (%d)\n", GETSOCKETERRNO());
		CLOSESOCKET(socket_listen);
		return -1;
	}
	const int opt = 1;
	if (setsockopt(socket_listen, SOL_SOCKET, SO_REUSEADDR,
		(char*)&opt, sizeof(opt)) == -1) {

		std::cerr << "Could not set socket options (" << GETSOCKETERRNO() << ")" << std::endl;
		CLOSESOCKET(socket_listen);
		return -1;
	}

	// Set the socket to non-blocking mode
#if defined(_WIN32)
	u_long mode = 1;
	if (ioctlsocket(socket_listen, FIONBIO, &mode) != NO_ERROR) {
		std::cerr << "ioctlsocket() failed. (" << GETSOCKETERRNO() << ")\n";
		CLOSESOCKET(socket_listen);
		return -1;
	}
#else
	int flags = fcntl(socket_listen, F_GETFL, 0);
	if (flags == -1) {
		std::cerr << "fcntl(F_GETFL) failed. (" << errno << ")\n";
		CLOSESOCKET(socket_listen);
		return -1;
	}
	if (fcntl(socket_listen, F_SETFL, flags | O_NONBLOCK) == -1) {
		std::cerr << "fcntl(F_SETFL) failed. (" << errno << ")\n";
		CLOSESOCKET(socket_listen);
		return -1;
	}
#endif

	printf("Binding socket to local address...\n");
	if (bind(socket_listen,
		bind_address->ai_addr, bind_address->ai_addrlen)) {
		std::cerr << "bind() failed. (" << GETSOCKETERRNO() << ")\n";
		CLOSESOCKET(socket_listen);
		return -1;
	}
	freeaddrinfo(bind_address);

	printf("Listening...\n");
	if (listen(socket_listen, 10) < 0) {
		std::cerr << "listen() failed. (" << GETSOCKETERRNO() << ")\n";
		CLOSESOCKET(socket_listen);
		return -1;
	}
	return socket_listen;
}

//int Server::select_read(SOCKET server_socket) {
	//	fd_set master_set;
	//
	//	//,read_set;
	//	FD_ZERO(&master_set);
	//	FD_SET(server_socket, &master_set);
	//	struct timeval timeout;
	//	timeout.tv_sec = static_cast<long>(0); ;
	//	timeout.tv_usec = static_cast<decltype(timeout.tv_usec)>(10000);
	//	return handle_EINTR([&]() {
	//		return select(static_cast<int>(server_socket + 1), &master_set, nullptr, nullptr, &timeout);
	//		});
	//}

//void Server::handle_new_connection(const SOCKET server_socket, fd_set& master_set, SOCKET& maxSock) {
//	SOCKET client_sock = accept(server_socket, nullptr, nullptr);
//	if (!ISVALIDSOCKET(client_sock)) {
//		std::cerr << "accept failed: " << errno << std::endl;
//		return;
//	}
//	std::cout << "Client connected." << std::endl;
//
//	{
//		std::lock_guard<std::mutex> lock(fd_set_mutex);
//		FD_SET(client_sock, &master_set);
//		if (client_sock > maxSock) {
//			maxSock = client_sock;
//		}
//	}
//
//	// Send registration or login prompt to the new client
//	std::string prompt = "Welcome! Please register or login:\n";
//	send(client_sock, prompt.c_str(), prompt.size(), 0);
//}