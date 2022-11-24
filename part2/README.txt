


		
/--------------Server & Client program are successful to run on below command-------------------/
/-----------------------------------------------------------------------------------------------/
Name                                                            Description 
/login <client ID> <Password> <server-IP> <server-port>         Initial with client, and try to make a connection with server. 
/logout                                                         Logout from the server, but not close the program
/joinsession <session ID>                                       Join with new session, if no found, create a new one.
/leavesession <session ID>                                      Leave the session
/createsession <session ID>                                     Create the session
/list                                                           Show the client list and conference list of current state on server.
/quit                                                           Close the client program
9                                                               Get the self client information
/invite <client name> <conference name>                         invite client to join with the session (yes)=>accept to join , (no)=>reject
/-----------------------------------------------------------------------------------------------/


-- Overview: 
	- Introduction
		After the testing, we realise that the program are only allow 5 clients at maximum to connect with server, and the command is strictly follow by the it's format. 
		To be more specific, the command is space sensitive. For example, "/createsession Hello1 " is differ than "/createsession Hello1", and the previous one might even cause segmentation fault.
		For the invitation part, the client might not receive the message of invitation, so that it needs to rerun the command again. It usually work after run the (/invite .. ..) 3 times.		
	- Objective
		Below are some demostration of showing two properties
			- one client is able to invite with other client
			- one client can join with multiple conferences
		
	- Server Status
		To see the status of server, User can type the code (/list) or simply (5) on any client. 
		
		After typing, the server, will gives feedback of current (Client list) and (conference list), which will similar like this:
										------ Client List ------
										No.     SockID  Name    Password
										0       4       jill    eW94dsol
										1       5       Gina    easfasdfal
										2       6       Daniel  1234bd
										3       7       Joe     1234bd
										4       8       Jerry   1234bd

										----------END-----------

										------ Conference List ------
										No.     Name    Client_list
										0       Hello1          jill    Jerry
										1       Hello2          Daniel  Joe

										----------END-----------
		Client List indicate the all clients that currently connected to the server
		Conference List indicate the all conference that currently open. if no conference, it will only appear "Empty" within the content 
		
	- Communication
		During the communication with each client process,  the client will receive the message like below
				Terminal 2: >>> Hi this is Gina speaking 
				Terminal 1: >>> Server: Client(jill): Hi this is Gina speaking
		it 
	
-- Example 1:       
	Description
		Client1(Jill) invites Client2(Gina) to join the conference(Hello).
		// inside the '( )' is the real name that we will use on this example
	Instruction
		This example use 3 Terminal for operate(Terminal_1 => Server, Terminal_2 => Client(Jill), Terminal_3 => Client(Gina))
		All the code below should be strictly write on indicating terminal, otherwise some error would appear.
	Procedure 
		
		Terminal_1: make 			// first build the server.c and client.c
		Terminal_1: ./server 9980	// run the server with port 9980
		
		// initial client 
		Terminal_2: ./client 												// initial with the client(Jill)
		Terminal_2: /login jill eW94dsol 127.0.0.1 9980		
		
		Terminal_3: ./client												// initial with the client(Gina)
		Terminal_3: /login Gina easfasdfal 127.0.0.1 9980					
		Terminal_3: /list
		
		// create conference and incite Gina
		Terminal_2: /createsession Hello
		Terminal_2: /invite Gina Hello										// Important, invitation might need to run 2 or more times, as long as it appears on the terminal
																			// Jill invite Gina to join the conference Hello
																			
		Terminal_3: yes / no												// Gina will accept or reject the invitation

	Important: 
		Sometimes the client might need to invite 2 or even more times, and therefore the invitation message will appear on that terminal
		During the procedure above, people can use (/list) to see the progress of the server status, and also can use (9) to see the client status.
		It is fine if the client doesn't response to the invitation, and 
	Result
		After running the procedure step, you could run (/list) on either client in order to see that client(Gina) has been join into conference(Hello).
		Besides, these two client are able to communicate with each other by type any text into the code(except number and '/' in the first word )
	
-------End of Example 1 ---------



-- Example 2
	Description
		Create 5 clients with different name, and 1 clients will join into first confernece(Hello1), and the other 2 clients will join into second conference(Hello2).
		one client will join with both client.
		The purpose of this example is to see that client_5(Jerry) can see the information from two conference, but other clients can only see the information at most one conference.
	Instruction
		Here is the overview of initialise the server, client and conference. 
			Terminal_1 => server
			Terminal_2 => client_1 => Jill => createsession (Hello1)
			Terminal_3 => client_2 => Gina 									// does not join with any conference, it use for Example 3
			Terminal_4 => client_3 => Daniel => createsession(Hello2)
			Terminal_5 => client_4 => Joe => joinsession(Hello2)
			Terminal_6 => client_5 => Jerry	=>both joinsession(Hello1) &  joinsession(Hello2)
		
	Procedure
		// initial server
		Terminal_1: make 			// first build the server.c and client.c
		Terminal_1: ./server 9980	// run the server with port 9980
		
		// initial clients 
		Terminal_2: ./client 												// initial with the client(Jill)
		Terminal_2: /login jill eW94dsol 127.0.0.1 9980		
		
		Terminal_3: ./client												// initial with the client(Gina)
		Terminal_3: /login Gina easfasdfal 127.0.0.1 9980
		
		Terminal_4: ./client												// initial with the client(Daniel)
		Terminal_4: /login Daniel 1234bd 127.0.0.1 9980
		
		Terminal_5: ./client												// initial with the client(Joe)
		Terminal_5: /login Joe 1234bd 127.0.0.1 9980
		
		Terminal_6: ./client												// initial with the client(Jerry)
		Terminal_6: /login Jerry 1234bd 127.0.0.1 9980
		
		// create & join conferences
		Terminal_2: /createsession Hello1									// create and join conference Hello1
		Terminal_4: /createsession Hello2									// create and join conference Hello2
		Terminal_5: /joinsession Hello2									// join conference Hello2
		Terminal_6: /joinsession Hello1
		Terminal_6: /joinsession Hello2
		
	Result
		Jerry can talk with all client by applying two conference. Except for Jerry,
		
			conference(Hello1):  Jill <--> Jerry 
			conference(Hello2): Daniel <--> Joe <--> Jerry
		
		Below is the result that I run.
			1. If we Type text on Terminal_6, the text will appear on all terminal. 
				Example: 
					Terminal_6: Hi This is Jerry, I will talk to all of you, except Gina.
					
			2. If we type text on either Terminal_2, the text will deliver to only Terminal 2, 6 (Jill, Jerry). 
			   On the other word, Terminal 4 and 5 (Daniel, and Joe) would not hear any information.
				Example:
					Terminal_2: Hi Jerry, This is Jill, and Nise to meet you
			3. Similarly, if we type text on either Terminal_4 or Terminal_5, the text will deliver to only Terminal 4, 5, 6 (Daniel, Joe, Jerry). 
				Example:
					Terminal_5: Hi, this is joe. Jill, and Gina cannot hear me.		

-------End of Example 2 ---------


-- Example 3
	Description
		This is the extension from example 2. we want to invite client(Gina) to join the conference by any existing clients.
		
		
	Instruction
		suppose Jill want to invite Gina
		As a result:
			conference(Hello1):  Jill <--> Gina <--> Jerry
			conference(Hello2): Daniel <--> Joe <--> Jerry
	Procedure
		Terminal_3: /list
		Terminal_2: /invite Gina Hello1
		Terminal_3: yes/no 
		
		
		
	Result
		At this moment, Jill, Gina, Jerry, Tom can talk each other. Daniel, joe, jerry can talk each other.
		
		
-------End of Example 3 ---------	