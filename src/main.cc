#include "common.hh"
#include "crypto.hh"
#include "login_server.hh"
#include "game_server.hh"

static
RSA *rsa_default_init(void){
	// TODO: Eventually have the private key in a PEM file
	// and have a function to load it.
	//	RSA *server_rsa = rsa_load_from_file("key.pem");

	static const char p[] =
		"142996239624163995200701773828988955507954033454661532174705160829"
		"347375827760388829672133862046006741453928458538592179906264509724"
		"52084065728686565928113";
	static const char q[] =
		"763097919597040472189120184779200212553540129277912393720744757459"
		"669278851364717923533552930725135057072840737370556470887176203301"
		"7096809910315212884101";
	static const char e[] = "65537";

	RSA *r = rsa_alloc();
	if(!rsa_setkey(r, p, q, e))
		PANIC("failed to set RSA key");
	return r;
}

#if BUILD_DEBUG
static
void calc_stats(i64 *data, int datalen,
		i64 *out_avg, i64 *out_min, i64 *out_max){
	ASSERT(datalen > 0);
	i64 avg, min, max;
	avg = min = max = data[0];
	for(int i = 1; i < datalen; i += 1){
		if(data[i] < min)
			min = data[i];
		else if(data[i] > max)
			max = data[i];
		avg += data[i];
	}
	*out_avg = avg / datalen;
	*out_min = min;
	*out_max = max;
}

static
void frame_stats(i64 frame_start, i64 frame_end, i64 next_frame){
	static i64 frame_user_time[120];
	static i64 frame_idle_time[120];
	static int num_frames = 0;
	if(num_frames >= 120){
		i64 avg, min, max;
		LOG("frame stats over %d frames", num_frames);
		calc_stats(frame_user_time, num_frames, &avg, &min, &max);
		LOG("    user time: avg = %lld, min = %lld, max = %lld", avg, min, max);
		calc_stats(frame_idle_time, num_frames, &avg, &min, &max);
		LOG("    idle time: avg = %lld, min = %lld, max = %lld", avg, min, max);
		num_frames = 0;
	}
	frame_user_time[num_frames] = frame_end - frame_start;
	frame_idle_time[num_frames] = (next_frame > frame_end) ? (next_frame - frame_end) : 0;
	num_frames += 1;
}
#endif

#if !BUILD_TEST
int main(int argc, char **argv){
	// TODO: Move these to a configuration file.
	usize arena_vsize = 0x100000000ULL; // ~4GB
	usize arena_granularity = 0x00400000UL; // ~4MB
	// game frame interval in milliseconds:
	//	16 is ~60fps
	//	33 is ~30fps
	//	66 is ~15fps
	i64 game_frame_interval = 33;


	MemArena *arena = arena_init(arena_vsize, arena_granularity);
	RSA *server_rsa = rsa_default_init();
	LoginServer *lserver = login_server_init(arena, server_rsa, 7171, 10);
	GameServer *gserver = game_server_init(arena, server_rsa, 7172, 100);

	while(1){
		i64 frame_start = sys_clock_monotonic_msec();
		i64 next_frame = frame_start + game_frame_interval;

		login_server_poll(lserver);
		game_server_poll(gserver);

		i64 frame_end = sys_clock_monotonic_msec();
		if(frame_end < next_frame)
			sys_sleep_msec(next_frame - frame_end);

#if 0 && BUILD_DEBUG
		frame_stats(frame_start, frame_end, next_frame);
#endif
	}
	return 0;
}
#else //BUILD_TEST
int main(int argc, char **argv){
	static const char *test_strings[] = {
		"hello",
		"ABCDEFGH",
		"abcd",
		"QWERTYUIOP",
	};

	usize len;
	u8 buf[128];
	RSA *rsa = rsa_default_init();
	for(i32 i = 0; i < NARRAY(test_strings); i += 1){
		debug_printf("RSA test #%d: \"%s\"\n", i, test_strings[i]);
		len = strlen(test_strings[i]);
		memcpy(buf, test_strings[i], len);

		ASSERT(rsa_encode(rsa, buf, &len, sizeof(buf)));
		debug_print_buf("encoded", buf, (i32)len);

		ASSERT(rsa_decode(rsa, buf, &len, sizeof(buf)));
		debug_print_buf("decoded", buf, (i32)len);

		debug_printf("\n");
	}
	rsa_free(rsa);
}
#endif //BUILD_TEST
