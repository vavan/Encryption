#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <stdio.h>
#include <string.h>

#include <iostream>
#include "connection.h"
#include "security.h"


using namespace std;

void test_connection()
{
	Socket s = Socket();
	s.connect("127.0.0.1", 8080);
	char hi[] = "Slava Ukraini!";
	cout << "Sent: " << s.send(hi, strlen(hi)) << endl;

}

void test_security()
{
	Security sec = Security();
	sec.build();

	char msg[] = "TEST ENCODING2";
	int len = sec.encrypt(msg, strlen(msg)+1);
	sec.decrypt(sec.encrypted, len);

	sec._free();
}


const char hi[] = "Slava Ukraini!";

char* create_hi(char *out, char* pub_key)
{
	strcpy(out, hi);

	int i = strlen(hi);

	int size = strlen(pub_key)-1;
	out[i++] = (char)(size>>8);
	out[i++] = (char)(size & 0xFF);

	strcpy(out+i, pub_key);

	return out;
}


int main(void) {
	Security sec = Security();
	sec.build();

	Socket s = Socket();
	s.connect("127.0.0.1", 8080);
//	cout << "Sent: " << s.send(hi, strlen(hi)) << endl;

	int hi_size = strlen(hi) + 2 + strlen(sec.pub_key);
	char* hi_msg = (char*)malloc(hi_size);

	cout << "Pub:" << sec.pub_key << endl;

	create_hi(hi_msg, sec.pub_key);
//	cout << endl << "***" << hi_msg << endl;
	int sent = s.send(hi_msg, hi_size-1);

	cout << "Sent: " << sent << endl;

	char* buffer = (char *)malloc(16384);
	int recv = s.recv(buffer, 16384);

	cout << "Recv: " << recv << endl;

	char ack[] = "Heroyam Slava!";
	sent = s.send(ack, strlen(ack));

	cout << "Sent2: " << sent << endl;


	free(hi_msg);
	sec._free();
    return 0;
}

