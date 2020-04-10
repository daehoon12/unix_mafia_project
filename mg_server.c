#include "mg_header.h"



int client_num = 4; // 기본 값.
int discussion_time = 10; // 기본 10초.
int killnum = -1; // 죽는 자의 번호.

struct mymsgbuf mesg;
int msgid, len;
key_t key;
key_t sh_key;
// 메세지 관련 변수들.

struct sigaction act;
// sigaction 구조체 포인터 변수.

FILE *rfp, *wfp;
// 파일 구조체 포인터

char * nameofjobs[5]={"host","mafia","citizen",};
// 직업 명 문자열을 가리키는 포인터 배열.

int nopb_jobs[5] = {0,}; // 직업별 사람수. 
//0: host, 1: mafia, 2 : citizen ...

int p_pids[MAX_PEOPLE] ; // people pids.
char * p_nics[MAX_PEOPLE]={0,}; // peple nicknames
int p_jobs[MAX_PEOPLE] ; // people jobs
// 죽은 면 -1 로 저장하자.

int b_p_jobs[MAX_PEOPLE]; // people jobs 의 backup 본.

void print_start_message();
void receive_print_message(int mtype);
void send_message(int mtype, char * message);
void random_job_make(int client_num, int arr[MAX_PEOPLE]);
void test_job_make();
void send_receive_first_message();

void write_user_data(); // 데이터를 실제로 쓰는 부분. 
void save_user_data_send_messages();
void save_final_data();


void print_kill_one_check_condition();
int check_end_condition();
// void print_kill~ 에서 쓰이는 함수. 종료 조건에 도달하면 1을 반환. 아니면 

void become_night();
void become_day(); 


// 함수의 선언부.


void print_start_message(){
	printf("안녕하세요 마피아 게임입니다.\n");
	printf("당신은 게임의 사회자로 선정되었습니다.\n");
	printf("제작 자 : 강대훈, 이동석.\n");
	printf("접속 할 사람수는 몇명입니까?\n");
	printf("입력 : ");
}

void receive_print_message(int mtype){
	len = msgrcv(msgid, &mesg, 800, mtype, 0);
	printf("Received Msg = %s \n", mesg.mtext);
}

void send_message(int mtype, char * message){
	mesg.mtype = mtype;
	strcpy(mesg.mtext, message);
	if (msgsnd(msgid, (void *)&mesg, 800, IPC_NOWAIT) == -1) {
		perror("msgsnd");
		exit(1);
	}
}


void random_job_make(int client_num, int arr[MAX_PEOPLE]){
	for(int i=0;i<client_num;i++)
	{
		arr[i]=2;
	}
	
	int cnt = 0;
	int temp=0;
	srand(time(NULL));
	
	if(client_num<=5)
	{
		int r = rand()%client_num;
		arr[r]=1;

	}
	else if(client_num>5)
	{
	
		int r = rand()%client_num;
		arr[r]=1;
		temp = r;
		cnt++;
		while(1)
		{
			r = rand()%client_num;
			if(cnt==2)
				break;
			if(temp == r)
			{
				continue;
			}
			arr[r]=1;
			temp = r;
			cnt++;
		}
	}
}

void test_job_make(){
	for(int i=0;i<client_num;i++){
		if(i==0)
			p_jobs[i] = Mafia;
		else
			p_jobs[i] = Citizen;
	}
}

void send_receive_first_message(){
	key = ftok("mg", 1); // mg means "mafia game".
	// key를 만듭니다.
	if ((msgid = msgget(key, IPC_CREAT|0644)) < 0) {
		perror("msgget");
		exit(1);
	}
	// 해당 키 값으로 message 식별자를 만듭니다.
	// 사람들 직업번호가 저장된 배열.
	nopb_jobs[Host] = 1;
	// 사회자는 항상 하나

	if(client_num<=5){
		nopb_jobs[Mafia] = 1;
		nopb_jobs[Citizen] = client_num -1;
	}
	if( client_num>=6){
		nopb_jobs[Mafia] =2;
		nopb_jobs[Citizen] = client_num -2;
	}
	//인원수에 따라 마피아, 시민수를 조절하는 과정입니다.

	//test_job_make()
	random_job_make(client_num, p_jobs);
	// 클라이언트의 수에 따라 랜덤하게 마피아와 시민을 정하는 코드 입니다.

	for(int i=0;i<client_num;i++){
		 b_p_jobs[i] =p_jobs[i];
		 // 백업 해두기.
	}
	// b_p_jobs 에 p_jobs 에 있던 값을 저장합니다.
	// 그 이유는 특정 i 번호에 해당하는 프로세스가 죽으면, 해당 사람의 직업이 -1이 되기 때문입니다.

	for(int i=0;i<MAX_PEOPLE;i++){
		p_nics[i] = malloc(sizeof(char) * 50);
	}
	// 닉네임을 저장할 공간을 할당하는 과정.

	int mafia_flag = 0 ; // 처음은 클라이언트.

	for(int i=0;i<client_num;i++){

		receive_print_message(1);
		char * strptr = strtok(mesg.mtext, "-");
		strcpy(p_nics[i],strptr);
		strptr = strtok(NULL, "-");
		p_pids[i] = atoi(strptr);
		// 해당 메세지를 받아, 거기서 nicsname과 pid를 추출하는 과정.

		memset(mesg.mtext, 0, sizeof(mesg.mtext));
		sprintf(mesg.mtext, "%d", i+3);
		send_message(2, mesg.mtext);
		// 참여자에게 관련 참여자가 사용할 mtype 번호를 메세지큐에 보냅니다.
		// 이때는 타입 2번 메세지를 사용하여 보내고
		//참여자들은 3번부터 mtype을 부여받게 됩니다. 

		memset(mesg.mtext, 0, sizeof(mesg.mtext));
		sprintf(mesg.mtext, "%d", p_jobs[i]);
		send_message(i+3, mesg.mtext);
		// mtype i+3 번으로 직업번호를 보내주자.
		// 해당 프로세스의 mtype으로 직업번호를 보냅니다.


		if(p_jobs[i] == Mafia){
			memset(mesg.mtext, 0, sizeof(mesg.mtext));
			sprintf(mesg.mtext, "%d", nopb_jobs[Mafia]);
			send_message(i+3, mesg.mtext);
			// 보낸 프로세스의 직업이 마피아라면 
			// 마피아에게 보낼  첫번째 메세지 : 마피아 수.
			
			if(nopb_jobs[Mafia] >=2){
				// 처음에 보내는 것은 당신이 mafia_client인가? mafia_server인가?
				// mafia_client면 0, mafia_server면 1. 
				memset(mesg.mtext, 0, sizeof(mesg.mtext));
				sprintf(mesg.mtext, "%d", mafia_flag);
				send_message(i+3, mesg.mtext);
				// 처음은 0보내고, 클라이언트.
				//두번쨰 녀석은 1. 서버
				// 마피아에게 보낼  두번째 메세지 : 마피아 서버인가 클라이언트 인가?.

				//그다음 메세지.
				mafia_flag =1;
				char mafia_info[500] = {0,};
				strcpy(mafia_info, "마피아 정보입니다.\n");
				strcat(mafia_info, "처음에만 알려주니 잘 기억하세요.\n");
				strcat(mafia_info, "번호\tnickname\n");
				for(int j=0;j<client_num;j++){
					if(p_jobs[j] == Mafia){
						char numstr[10] = {0,};
						sprintf(numstr, "%d", j);
						strcat(mafia_info, numstr);
						strcat(mafia_info,"\t");
						strcat(mafia_info, p_nics[j]);
						strcat(mafia_info, "\n");
					}
				}

				send_message(i+3, mafia_info);
				// 마피아에게 보낼  세번째 메세지 : 마피아 정보.
			}

			// 여기서 마피아가 두명 이상일 때 어떻게 처리할 지 고민해야 할 것이다.
		}
	}
}

void write_user_data(){
	// 사용자들에게 공지할 자료 입니다.
	// process에 할당된 index 번호, nickname, pid , 살아있는 지 유무를 알수 있습니다.
	// 그런한 자료를 만드는 과정.

	printf("\n\nuser_data.txt 갱신을 시작합니다.\n");
	if ((wfp = fopen("user_data.txt", "w")) == NULL) {
        perror("fopen: user_data.txt");
        exit(1);
	}
	fprintf(wfp,"-------user_data_file------\n");
	fprintf(wfp,"--num nic pid -----\n");
	for(int i=0;i<client_num;i++){
		if(p_jobs[i] == -1){
			fprintf(wfp,"%d : %s %d DEAD\n", i, p_nics[i], p_pids[i]);
		}
		else {
			fprintf(wfp,"%d : %s %d ALIVE\n", i, p_nics[i], p_pids[i]);
		}
	}
	fclose(wfp);

	printf("user_data.txt 갱신되었습니다!\n");
}

void save_user_data_send_messages(){
	write_user_data();
	// 사용자들에게 공지할 자료 입니다.
	// process에 할당된 index 번호, nickname, pid , 살아있는 지 유무를 알수 있습니다.
	// 그런한 자료를 만드는 과정.

	for(int i=0;i<client_num;i++){
		if(p_jobs[i] == -1){
			continue;
		}
		send_message(i+3, "User_data is sent by host");
	}
	// 자료가 만들어졌다고 알려주는 과정.
}

void save_final_data(){
	printf("save final data is called\n");
	if ((wfp = fopen("user_data.txt", "w")) == NULL) {
        perror("fopen: user_data.txt");
        exit(1);
	}
	fprintf(wfp,"-------user_data_file------\n");
	fprintf(wfp,"--num nic pid job-----\n");
	for(int i=0;i<client_num;i++)
	{
		if(p_jobs[i] == -1){
			fprintf(wfp,"%d : %s %d %s DEAD\n", i, p_nics[i], p_pids[i], nameofjobs[b_p_jobs[i]]);
		}
		else {
			fprintf(wfp,"%d : %s %d %s ALIVE\n", i, p_nics[i], p_pids[i], nameofjobs[b_p_jobs[i]]);
		}
	}

	fclose(wfp);
}

int check_end_condition(){
	if(nopb_jobs[Citizen]<=nopb_jobs[Mafia]){
		save_final_data();
		// 게임이 끝나서 직업 까지 알수 있는 정보를 게시.
		// 마피아 승리 조건.
		for(int i=0;i<client_num;i++){
			if(p_jobs[i] == -1){
				continue;
			}
			kill((pid_t)p_pids[i], SIGUSR1);
		}
		// 모든 client에게 SIGUSR1을 보냄.
		// 마피아의 승리를 의미.
		return 1;
	}
	else if(nopb_jobs[Mafia] <=0){
		save_final_data();
		// 시민승리 조건.
		for(int i=0;i<client_num;i++){
			if(p_jobs[i] == -1){
				continue;
			}
			kill((pid_t)p_pids[i], SIGUSR2);
			// 모든 client에게 SIGUSR2을 보냄.
			// 시민의 승리를 의미.
		}

		// 모든 client에게 signal을 보냄.
		return 1;
	}
	else
		return 0;
}

void print_kill_one_check_condition(){
	printf("죽은 사람은 %d 입니다.\n", killnum);
	nopb_jobs[p_jobs[killnum]]--;
	p_jobs[killnum] = -1; //  해당 번호를 죽은 것으로 표시.
	
	kill((pid_t)p_pids[killnum], SIGQUIT);
	// 해당 pid로 시그널 보내기.

	//check_end_condition
	// 끝나는 조건에 도달했는지 확인.
	if(check_end_condition() == 1){

		// 자신을  종료..
		exit(1);
	}
}

void become_night(){
	//2.1 모두에게 밤이 되었음을 알림.
	printf("밤이 되었습니다.\n모두 엎드려주세요.\n마피아는 고개를 들어 서로를 확인하시고\n토의시간을 거쳐 죽일 사람을 선택하세요.\n시민은 잠시 기다려주세요.\n사회자는 마피아의 선택을 기다립니다.\n");
	for(int i=0;i<client_num;i++){
		if(p_jobs[i] == -1){
			continue;
			// 무시.. 죽었으니까.
		}
		send_message(i+3, "밤이 되었습니다.\n모두 엎드려주세요.\n마피아는 고개를 들어 서로를 확인하시고\n토의시간을 거쳐 죽일 사람을 선택하세요.\n시민은 잠시 기다려주세요.\n");
	}

	//2.2 마피아가 신호를 보내주기를 기다림.
	for(int i=0;i<nopb_jobs[Mafia];i++){
		receive_print_message(1);
	} 

	//2.3 해당 넘버를 죽은 것으로 표시하고 종료 메세지를 보냄.
	killnum = atoi(mesg.mtext);
	// 여기서 

	print_kill_one_check_condition();
	// 1명을 죽이고 종료 조건을 체크함.
	for(int i=0;i<client_num;i++){
		if(p_jobs[i] == -1){
			continue;
			// 무시.. 죽었으니까.
		}
		if(p_jobs[i] == Mafia){
			memset(mesg.mtext, 0, sizeof(mesg.mtext));
			sprintf(mesg.mtext, "%d", nopb_jobs[Mafia]);
			send_message(i+3, mesg.mtext);
		}
		// 보내는 메세지 받는 사람이 마피아면 현재 마피아 수를 추가적으로 보내준다.
	}	

	write_user_data();
	// 낮이 되기 전에 최신 정보를 저장함.

}

int vote_result(int whowilldie[]){
	int maxindex = 0;
	int max = -1;
	int samecnt =0;
	for(int i=0;i<client_num;i++){
		if(p_jobs[i] == -1){
			continue;
		}
		if(max<whowilldie[i]){
			samecnt = 0;
			max = whowilldie[i];
			maxindex =i;
		}
		else if(max == whowilldie[i]){
			samecnt =1;
		}
	}
	if(max <= 0){
		return -1;
	}
	if(samecnt ==1){
		return -1;
	}

	return maxindex;
}

void become_day(){
// 3. 낮이 됨
// 사회자가 마피아가 누구를 죽였다라고 말해줌
// 남아있는 사람들끼리 정해진 시간동안 토론을 한다.
// 정해진 시간이 지나면 한명 씩 지목한다.
// 사회자는 투표 중에서 동률이나 무효표가 아니면
// 그 사람을 처형하고
// 누군가 투표에 의해 죽었는지 알려준다.
// (종료조건을 확인)
	printf("낮이 되었습니다.\n");

	//  공유 메모리 만들기.
	int shmid = shmget(sh_key, MEMORY_SIZE, IPC_CREAT|0644);
	if (shmid == -1) {
          perror("shmget");
          exit(1);
      }

	char day_message[300] = "낮이 되었습니다.\n 마피아는 밤에 ";
	strcat(day_message, p_nics[killnum]);
	strcat(day_message, "를 죽였습니다.\n정해진 ");
	char tempstr[10]={0,};
	sprintf(tempstr, "%d", discussion_time);
	strcat(day_message, tempstr);
	strcat(day_message, "초 시간동안 대화를 시작해주세요. \n");
	for(int i=0;i<client_num;i++){
		if(p_jobs[i] == -1){
			continue;
			// 무시.. 죽었으니까.
		}
		send_message(i+3, day_message);
	}

	sleep(discussion_time);
	// 정해진 시간 기다리기.
	printf("토론 시간이 종료되었습니다.\n 각 유저로 부터 I'm done메세지를 기다립니다.\n");
	for(int i=0;i<client_num;i++){
		if(p_jobs[i] == -1){
			continue;
		}
		kill((pid_t)p_pids[i], SIGALRM);
	}
	// 토론이 끝났다는 시그널 날리기
	// SIGALRM 메세지를 보낸다.

	sleep(1);
	// 1초 sleep

	
	for(int i=0;i<client_num;i++){
		if(p_jobs[i] == -1){
			continue; 
		} // 죽은 아이디는 기다리지 않는다.
		receive_print_message(1);
	}
	//I'm done 메세지 기다리기

	shmctl(shmid, IPC_RMID, NULL);
	// 공유 메모리 삭제하기
	printf("모든 done 메세지를 받았고, 공유 메모리를 삭제하였습니다. \n 투표를 진행합니다.\n");
	// 투표 시작하기.
	for(int i=0;i<client_num;i++){
		if(p_jobs[i] == -1){
			continue; 
		} // 죽은 아이디는 기다리지 않는다.
		send_message(i+3, "이제 투표를 시작합니다.! \n특정 유저의 번호를 지정해주세요!\n죽은사람은 투표시 무효표가 됩니다~!\n");
	}

	int whowilldie[client_num];
	memset(whowilldie, 0, sizeof(whowilldie));

	printf("사용자들로 부터 표를 기다립니다.");
	for(int i=0;i<client_num;i++){
		if(p_jobs[i] == -1){
			continue; 
		} // 죽은 아이디는 기다리지 않는다.
		receive_print_message(1);
		whowilldie[atoi(mesg.mtext)]++;
	}

	printf("투표가 성공적으로 치뤄졌습니다.\n");
	killnum = vote_result(whowilldie);
	printf("투표 결과가 나옵니다.\n");
	if(killnum != -1){
		// 아래에서 죽이고 종료조건을 체크함.
		print_kill_one_check_condition();
		write_user_data();
		// 선거 결과를 보내주기 전, 밤이 되기 전에 최신 데이터를 갱신.

		char day_killed_message[300] = "투표가 종료되었습니다.\n 선거 결과\n ";
		strcat(day_killed_message, p_nics[killnum]);
		strcat(day_killed_message, "를 죽였습니다.\n");

		// 좀 이상하지만, 죽었음 알리고, 
		for(int i=0;i<client_num;i++){
			if(p_jobs[i] == -1){
				continue;
			}
			send_message(i+3, day_killed_message);
			if(p_jobs[i] == Mafia){
				memset(mesg.mtext, 0, sizeof(mesg.mtext));
				sprintf(mesg.mtext, "%d", nopb_jobs[Mafia]);
				send_message(i+3, mesg.mtext);
			}
			// 보내는 메세지 받는 사람이 마피아면 현재 마피아 수를 추가적으로 보내준다.
		}

	}
	else{
		write_user_data();
		// 선거 결과를 보내주기 전, 밤이 되기 전에 최신 데이터를 갱신.
		
		for(int i=0;i<client_num;i++){
			if(p_jobs[i] == -1){
				continue;
			}
			send_message(i+3,"투표가 종료되었습니다. \n선거 결과 동률 또는 무효표로 인해 아무도 죽지 않았습니다.\n");
			// 동률 또는 무효표에 의해서 아무도 죽지 않았음을 명시.
			if(p_jobs[i] == Mafia){
				memset(mesg.mtext, 0, sizeof(mesg.mtext));
				sprintf(mesg.mtext, "%d", nopb_jobs[Mafia]);
				send_message(i+3, mesg.mtext);
			}
			// 보내는 메세지 받는 사람이 마피아면 현재 마피아 수를 추가적으로 보내준다.
		}
	}

}

int main(void){
	sh_key= ftok("shmfile", 1);
	// 공유 메모리를 위한 키 만들기.

// 1. 초기설정.
// 1-1. 접속 할 사람 수를 정하고, 사람 (process)의 접속을 기다린다.`
// 1-2. 사회자가 마피아를 랜덤으로 2명 선정
//1-3. 접속 정보를 저장하고 신호를 보내준다.
// (사회자 알려줘야 되요. 참여자 누구고 프로세스넘버 뭐있고, 닉네임뭐고.)

	print_start_message();
	scanf("%d", &client_num);
	printf("토론 시간을 입력 해주세요. : ");
	scanf("%d", &discussion_time);

	printf("참여자의 접속을 기다립니다.\n");

	send_receive_first_message();
// 위 함수에서 1-1 뒷부분과과 1-2 를 실행.
// 1-1.뒷 부분. 사람 (process)의 접속을 기다린다.`
// 1-2. 사회자가 마피아를 랜덤으로 2명 선정

	printf("test success\n");
	printf("-------user_data_file------\n");
	printf("--num nic pid -----\n");
	for(int i=0;i<client_num;i++){
		if(p_jobs[i] == -1){
			continue;
		}
		printf("%d : %s %d\n", i, p_nics[i], p_pids[i]);
	}
	save_user_data_send_messages();
//1-3. 접속 정보를 저장하고 신호를 보내준다.
// (사회자 알려줘야 되요. 참여자 누구고 프로세스넘버 뭐있고, 닉네임뭐고.)

	while(1){
// 2. 밤이 되었습니다.
// 마피아는 고개를 들어서 서로를 확인.
// 마피아 2명 이상이면 토의 시간 거치고
// 죽일 사람을 선택
// 그 자를 죽임.
// 종료 조건 확인.
		become_night();
// 3. 낮이 됨
// 사회자가 마피아가 누구를 죽였다라고 말해줌
// 남아있는 사람들끼리 정해진 시간동안 토론을 한다.
// 정해진 시간이 지나면 한명 씩 지목한다.
// 사회자는 투표 중에서 동률이나 무효표가 아니면
// 그 사람을 처형하고
// 누군가 투표에 의해 죽었는지 알려준다.
// (종료조건을 확인)
		become_day();
	}
// 4. 2-3을 반복.

}
