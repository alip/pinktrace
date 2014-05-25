#define SYSCALL_OFFSET 1024
	"ni_syscall", /* 1024 */
	"exit", /* 1025 */
	"read", /* 1026 */
	"write", /* 1027 */
	"open", /* 1028 */
	"close", /* 1029 */
	"creat", /* 1030 */
	"link", /* 1031 */
	"unlink", /* 1032 */
	"execve", /* 1033 */
	"chdir", /* 1034 */
	"fchdir", /* 1035 */
	"utimes", /* 1036 */
	"mknod", /* 1037 */
	"chmod", /* 1038 */
	"chown", /* 1039 */
	"lseek", /* 1040 */
	"getpid", /* 1041 */
	"getppid", /* 1042 */
	"mount", /* 1043 */
	"umount", /* 1044 */
	"setuid", /* 1045 */
	"getuid", /* 1046 */
	"geteuid", /* 1047 */
	"ptrace", /* 1048 */
	"access", /* 1049 */
	"sync", /* 1050 */
	"fsync", /* 1051 */
	"fdatasync", /* 1052 */
	"kill", /* 1053 */
	"rename", /* 1054 */
	"mkdir", /* 1055 */
	"rmdir", /* 1056 */
	"dup", /* 1057 */
	"pipe", /* 1058 */
	"times", /* 1059 */
	"brk", /* 1060 */
	"setgid", /* 1061 */
	"getgid", /* 1062 */
	"getegid", /* 1063 */
	"acct", /* 1064 */
	"ioctl", /* 1065 */
	"fcntl", /* 1066 */
	"umask", /* 1067 */
	"chroot", /* 1068 */
	"ustat", /* 1069 */
	"dup2", /* 1070 */
	"setreuid", /* 1071 */
	"setregid", /* 1072 */
	"getresuid", /* 1073 */
	"setresuid", /* 1074 */
	"getresgid", /* 1075 */
	"setresgid", /* 1076 */
	"getgroups", /* 1077 */
	"setgroups", /* 1078 */
	"getpgid", /* 1079 */
	"setpgid", /* 1080 */
	"setsid", /* 1081 */
	"getsid", /* 1082 */
	"sethostname", /* 1083 */
	"setrlimit", /* 1084 */
	"getrlimit", /* 1085 */
	"getrusage", /* 1086 */
	"gettimeofday", /* 1087 */
	"settimeofday", /* 1088 */
	"select", /* 1089 */
	"poll", /* 1090 */
	"symlink", /* 1091 */
	"readlink", /* 1092 */
	"uselib", /* 1093 */
	"swapon", /* 1094 */
	"swapoff", /* 1095 */
	"reboot", /* 1096 */
	"truncate", /* 1097 */
	"ftruncate", /* 1098 */
	"fchmod", /* 1099 */
	"fchown", /* 1100 */
	"getpriority", /* 1101 */
	"setpriority", /* 1102 */
	"statfs", /* 1103 */
	"fstatfs", /* 1104 */
	"gettid", /* 1105 */
	"semget", /* 1106 */
	"semop", /* 1107 */
	"semctl", /* 1108 */
	"msgget", /* 1109 */
	"msgsnd", /* 1110 */
	"msgrcv", /* 1111 */
	"msgctl", /* 1112 */
	"shmget", /* 1113 */
	"shmat", /* 1114 */
	"shmdt", /* 1115 */
	"shmctl", /* 1116 */
	"syslog", /* 1117 */
	"setitimer", /* 1118 */
	"getitimer", /* 1119 */
	"stat", /* 1120 */
	"lstat", /* 1121 */
	"fstat", /* 1122 */
	"vhangup", /* 1123 */
	"lchown", /* 1124 */
	"vm86", /* 1125 */
	"wait4", /* 1126 */
	"sysinfo", /* 1127 */
	"clone", /* 1128 */
	"setdomainname", /* 1129 */
	"uname", /* 1130 */
	"adjtimex", /* 1131 */
	"create_module", /* 1132 */
	"init_module", /* 1133 */
	"delete_module", /* 1134 */
	"get_kernel_syms", /* 1135 */
	"query_module", /* 1136 */
	"quotactl", /* 1137 */
	"bdflush", /* 1138 */
	"sysfs", /* 1139 */
	"personality", /* 1140 */
	"afs_syscall", /* 1141 */
	"setfsuid", /* 1142 */
	"setfsgid", /* 1143 */
	"getdents", /* 1144 */
	"flock", /* 1145 */
	"readv", /* 1146 */
	"writev", /* 1147 */
	"pread", /* 1148 */
	"pwrite", /* 1149 */
	"_sysctl", /* 1150 */
	"mmap", /* 1151 */
	"munmap", /* 1152 */
	"mlock", /* 1153 */
	"mlockall", /* 1154 */
	"mprotect", /* 1155 */
	"mremap", /* 1156 */
	"msync", /* 1157 */
	"munlock", /* 1158 */
	"munlockall", /* 1159 */
	"sched_getparam", /* 1160 */
	"sched_setparam", /* 1161 */
	"sched_getscheduler", /* 1162 */
	"sched_setscheduler", /* 1163 */
	"sched_yield", /* 1164 */
	"sched_get_priority_max", /* 1165 */
	"sched_get_priority_min", /* 1166 */
	"sched_rr_get_interval", /* 1167 */
	"nanosleep", /* 1168 */
	"nfsservctl", /* 1169 */
	"prctl", /* 1170 */
	"getpagesize", /* 1171 */
	"mmap2", /* 1172 */
	"pciconfig_read", /* 1173 */
	"pciconfig_write", /* 1174 */
	"perfmonctl", /* 1175 */
	"sigaltstack", /* 1176 */
	"rt_sigaction", /* 1177 */
	"rt_sigpending", /* 1178 */
	"rt_sigprocmask", /* 1179 */
	"rt_sigqueueinfo", /* 1180 */
	"rt_sigreturn", /* 1181 */
	"rt_sigsuspend", /* 1182 */
	"rt_sigtimedwait", /* 1183 */
	"getcwd", /* 1184 */
	"capget", /* 1185 */
	"capset", /* 1186 */
	"sendfile", /* 1187 */
	"getpmsg", /* 1188 */
	"putpmsg", /* 1189 */
	"socket", /* 1190 */
	"bind", /* 1191 */
	"connect", /* 1192 */
	"listen", /* 1193 */
	"accept", /* 1194 */
	"getsockname", /* 1195 */
	"getpeername", /* 1196 */
	"socketpair", /* 1197 */
	"send", /* 1198 */
	"sendto", /* 1199 */
	"recv", /* 1200 */
	"recvfrom", /* 1201 */
	"shutdown", /* 1202 */
	"setsockopt", /* 1203 */
	"getsockopt", /* 1204 */
	"sendmsg", /* 1205 */
	"recvmsg", /* 1206 */
	"pivot_root", /* 1207 */
	"mincore", /* 1208 */
	"madvise", /* 1209 */
	"stat", /* 1210 */
	"lstat", /* 1211 */
	"fstat", /* 1212 */
	"clone2", /* 1213 */
	"getdents64", /* 1214 */
	"getunwind", /* 1215 */
	"readahead", /* 1216 */
	"setxattr", /* 1217 */
	"lsetxattr", /* 1218 */
	"fsetxattr", /* 1219 */
	"getxattr", /* 1220 */
	"lgetxattr", /* 1221 */
	"fgetxattr", /* 1222 */
	"listxattr", /* 1223 */
	"llistxattr", /* 1224 */
	"flistxattr", /* 1225 */
	"removexattr", /* 1226 */
	"lremovexattr", /* 1227 */
	"fremovexattr", /* 1228 */
	"tkill", /* 1229 */
	"futex", /* 1230 */
	"sched_setaffinity",/* 1231 */
	"sched_getaffinity",/* 1232 */
	"set_tid_address", /* 1233 */
	"fadvise64", /* 1234 */
	"tgkill", /* 1235 */
	"exit_group", /* 1236 */
	"lookup_dcookie", /* 1237 */
	"io_setup", /* 1238 */
	"io_destroy", /* 1239 */
	"io_getevents", /* 1240 */
	"io_submit", /* 1241 */
	"io_cancel", /* 1242 */
	"epoll_create", /* 1243 */
	"epoll_ctl", /* 1244 */
	"epoll_wait", /* 1245 */
	"restart_syscall", /* 1246 */
	"semtimedop", /* 1247 */
	"timer_create", /* 1248 */
	"timer_settime", /* 1249 */
	"timer_gettime", /* 1250 */
	"timer_getoverrun", /* 1251 */
	"timer_delete", /* 1252 */
	"clock_settime", /* 1253 */
	"clock_gettime", /* 1254 */
	"clock_getres", /* 1255 */
	"clock_nanosleep", /* 1256 */
	"fstatfs64", /* 1257 */
	"statfs64", /* 1258 */
	"mbind", /* 1259 */
	"get_mempolicy", /* 1260 */
	"set_mempolicy", /* 1261 */
	"mq_open", /* 1262 */
	"mq_unlink", /* 1263 */
	"mq_timedsend", /* 1264 */
	"mq_timedreceive", /* 1265 */
	"mq_notify", /* 1266 */
	"mq_getsetattr", /* 1267 */
	"kexec_load", /* 1268 */
	"vserver", /* 1269 */
	"waitid", /* 1270 */
	"add_key", /* 1271 */
	"request_key", /* 1272 */
	"keyctl", /* 1273 */
	"ioprio_set", /* 1274 */
	"ioprio_get", /* 1275 */
	"move_pages", /* 1276 */
	"inotify_init", /* 1277 */
	"inotify_add_watch", /* 1278 */
	"inotify_rm_watch", /* 1279 */
	"migrate_pages", /* 1280 */
	"openat", /* 1281 */
	"mkdirat", /* 1282 */
	"mknodat", /* 1283 */
	"fchownat", /* 1284 */
	"futimesat", /* 1285 */
	"newfstatat", /* 1286 */
	"unlinkat", /* 1287 */
	"renameat", /* 1288 */
	"linkat", /* 1289 */
	"symlinkat", /* 1290 */
	"readlinkat", /* 1291 */
	"fchmodat", /* 1292 */
	"faccessat", /* 1293 */
	"pselect6", /* 1294 */
	"ppoll", /* 1295 */
	"unshare", /* 1296 */
	"set_robust_list", /* 1297 */
	"get_robust_list", /* 1298 */
	"splice", /* 1299 */
	"sync_file_range", /* 1300 */
	"tee", /* 1301 */
	"vmsplice", /* 1302 */
	NULL, /* 1303 */
	"getcpu", /* 1304 */
	"epoll_pwait", /* 1305 */
	NULL, /* 1306 */
	"signalfd", /* 1307 */
	"timerfd", /* 1308 */
	"eventfd", /* 1309 */
	"preadv", /* 1319 */
	"pwritev", /* 1320 */
	"rt_tgsigqueueinfo", /* 1321 */
	"recvmmsg", /* 1322 */
	"fanotify_init", /* 1323 */
	"fanotify_mark", /* 1324 */
	"prlimit64", /* 1325 */
	"name_to_handle_at", /* 1326 */
	"open_by_handle_at", /* 1327 */
	"clock_adjtime", /* 1328 */
	"syncfs", /* 1329 */
	"setns", /* 1330 */
	"sendmmsg", /* 1331 */
	"process_vm_readv", /* 1332 */
	"process_vm_writev", /* 1333 */
	"accept4", /* 1334 */
	"finit_module", /* 1335 */
