#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int32_t i32;
typedef size_t usize;

void *malloc_no_fail(usize size){
	void *mem = malloc(size);
	if(!mem) abort();
	return mem;
}

void print_last_error(const char *what){
	DWORD err_code = GetLastError();
	char err_str[256];
	DWORD ret = FormatMessageA(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, err_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		err_str, sizeof(err_str), NULL);
	if(ret == 0)
		strcpy(err_str, "unknown error");

	printf("%s | error(%d): %s\n", what, err_code, err_str);
}

void debug_print_buf_line(u8 *buf, i32 buflen){
	for(i32 i = 0; i < buflen; i += 1)
		printf("%02X ", buf[i]);
	printf(" | ");
	for(i32 i = 0; i < buflen; i += 1)
		printf("%c ", isprint(buf[i]) ? buf[i] : '.');

	printf("\n");
}

void debug_print_buf(u8 *buf, i32 buflen){
	i32 chars_per_line = 16;
	i32 lines = buflen / chars_per_line;
	for(i32 i = 0; i < lines; i += 1)
		debug_print_buf_line(&buf[i * chars_per_line], chars_per_line);
}

struct TibiaVersion{
	const char *version_str;
	u32 max_rsa_len;
	u32 max_host_len;
	u32 num_host_port_pairs;
	u32 host_port_stride;
	u8 *version_addr;
	u8 *rsa_addr;
	u8 *first_host_addr;
	u8 *first_port_addr;
};

void change_ip(TibiaVersion *v,
		const char *host, const char *port, const char *rsa){

	u32 host_len = (u32)strlen(host) + 1;
	if(host_len > v->max_host_len){
		printf("error: host_len > v->max_host_len\n");
		return;
	}

	u8 port_buf[2];
	u32 port_num = (u32)atoi(port);
	if(port_num > 0xFFFF){
		printf("error: invalid port\n");
		return;
	}
	port_buf[0] = (u8)port_num;
	port_buf[1] = (u8)(port_num >> 8);

	u32 rsa_len = (u32)strlen(rsa) + 1;
	if(rsa_len > v->max_rsa_len){
		printf("error: rsa_len > v->max_rsa_len\n");
		return;
	}
	if(v->rsa_addr != NULL && rsa_len == 0){
		printf("error: v->rsa_addr != NULL but rsa_len == 0\n");
		return;
	}

	HWND window = FindWindowA("TibiaClient", NULL);
	if(!window){
		print_last_error("FindWindowA");
		return;
	}

	DWORD process_id;
	GetWindowThreadProcessId(window, &process_id);

	HANDLE process = OpenProcess(
		PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, process_id);
	if(process == NULL){
		print_last_error("OpenProcess");
		return;
	}

	BOOL ret;

	{
		u8 version_buf[128];
		ret = ReadProcessMemory(process, v->version_addr,
			version_buf, sizeof(version_buf), NULL);
		if(!ret){
			print_last_error("VERSION ReadProcessMemory");
			printf("error: unable to check client version\n");
			return;
		}
		debug_print_buf(version_buf, sizeof(version_buf));

		u32 version_str_len = (u32)strlen(v->version_str);
		if(strncmp(v->version_str, (char*)version_buf, version_str_len) != 0){
			printf("error: invalid client version\n");
			return;
		}
	}

	for(u32 i = 0; i < v->num_host_port_pairs; i += 1){
		u8 *write_addr;

		write_addr = v->first_host_addr + i * v->host_port_stride;
		ret = WriteProcessMemory(process, write_addr, host, host_len, NULL);
		if(!ret) print_last_error("HOST WriteProcessMemory");

		write_addr = v->first_port_addr + i * v->host_port_stride;
		ret = WriteProcessMemory(process, write_addr, port_buf, 2, NULL);
		if(!ret) print_last_error("PORT WriteProcessMemory");
	}

	// NOTE: The RSA lives in read only memory which means we need to change
	// the protection flags before writing and restore them after.
	if(rsa_len > 0){
		DWORD old_protection, dummy;
		ret = VirtualProtectEx(process, v->rsa_addr,
			v->max_rsa_len, PAGE_READWRITE, &old_protection);
		if(!ret){
			print_last_error("RSA VirtualProtectEx CHANGE");
		}else{
			ret = WriteProcessMemory(process, v->rsa_addr, rsa, rsa_len, NULL);
			if(!ret) print_last_error("RSA WriteProcessMemory");

			ret = VirtualProtectEx(process, v->rsa_addr,
				v->max_rsa_len, old_protection, &dummy);
			if(!ret) print_last_error("RSA VirtualProtectEx RESTORE");
		}
	}

	CloseHandle(process);
}

int main(int argc, char **argv){
	const char *host = "localhost";
	const char *port = "7171";
	if(argc >= 2)
		host = argv[1];
	if(argc >= 3)
		port = argv[2];

	// NOTE: Need to keep in mind that the values of max_rsa_len and
	// max_host_len should already include the nul terminator.

	// NOTE: It seems that across multiple versions, the host_port_stride
	// is always 112 bytes with 100 bytes for the host name, 2 bytes for
	// the port and 10 bytes for something else (?). This means we don't
	// really need the first_port_addr since it is always at offset 100
	// from it's host name.

	TibiaVersion v860;
	v860.version_str = "Version 8.60";
	v860.max_rsa_len = 312;
	v860.max_host_len = 100;
	v860.num_host_port_pairs = 10;
	v860.host_port_stride = 112;
	v860.version_addr = (u8*)0x64C2AD;
	v860.rsa_addr = (u8*)0x5B8980;
	v860.first_host_addr = (u8*)0x7947F8;
	v860.first_port_addr = (u8*)0x79485C;

	const char *opentibia_rsa =
		"1091201329673994292788609605089955415282375029027981291234687579"
		"3726629149257644633073969600111060390723088861007265581882535850"
		"3429057592827629436413108566029093628212635953836686562675849720"
		"6207862794310902180176810615217550567108238764764442605581471797"
		"07119674283982419152118103759076030616683978566631413";


	change_ip(&v860, host, port, opentibia_rsa);
	return 0;
}
