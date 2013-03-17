#include "synchlist.h"

#define NumOFIpc 100

class Ipc{
	public:
		Ipc();
		~Ipc();
		int msgget(char* key);
		int msgsend(int id, char* msg);
		int msgrcv(int id, char* msg);
	private:
		SynchList* ipcQueue[NumOFIpc];
		int ipcHash[NumOFIpc];	
};

class Message{
	public:
		char* msg;
		int len;
		Message(char * _msg) {
			len = strlen(_msg);
			msg = new char[len+5];
			strcpy(msg, _msg);
		}
};

