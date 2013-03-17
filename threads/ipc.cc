#include "ipc.h"

Ipc::Ipc() {
	for (int i = 0; i < NumOFIpc; i++) {
		ipcQueue[i] = new SynchList;
		ipcHash[i] = 0;
	}
}

Ipc::~Ipc() {
	for (int i = 0; i < NumOFIpc; i++) {
		delete ipcQueue[i];
	}
}

int Ipc::msgget(char* key) {
	int hash = 0;
	int len = strlen(key);
	for (int i = 0; i < len; i++) {
		hash += key[i];
	}
	int id = hash % NumOFIpc;
	while (ipcHash[id] != 0 && ipcHash[id] != hash) {
		id = (id+1) % NumOFIpc;
	}
	ipcHash[id] = hash;
	return id;
}

int Ipc::msgsend(int id, char* msg) {
	Message *m = new Message(msg);
	ipcQueue[id]->Append((void*)m);
	return 0;
}

int Ipc::msgrcv(int id, char* msg) {
	Message* item = (Message*)ipcQueue[id]->Remove();
	strcpy(msg, item->msg);
	return 0;
}



