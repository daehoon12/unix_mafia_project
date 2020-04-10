#include "mg_header.h"

int mymtype;
char my_nickname[30]={0,};
int myjobnum;
int mypid;
int killnum = -1;
// 내 정보(메세지 타입, 닉네임, 직업, 내 pid)
//내가 죽일 사람 번호.

int msgid;
struct mymsgbuf mesg;
char buf[BUFSIZ];
int n;
key_t key;

// 메세지 관련 변수들.

char s_message[500] = {0,};
struct sockaddr_in sin, cli;
int sd, ns, clientlen = sizeof(cli);

struct sigaction act;
// sigaction 구조체 포인터 변수.
int discuss_end_flag = 0;
// 토론 종료를 판별하기 위한 플래그.
int mafia_flag = 0;
// 마피아 플래그 0은 클라. 1은 서버.

FILE *rfp, *wfp;
// 파일 구조체 포인터

char * nameofjobs[5]={"host","mafia","citizen",};
// 직업 명 문자열을 가리키는 포인터 배열.

char * my_data;
// 내가 보낼 초기 메세지.


int nopb_jobs[5] = {0,};
// 직업별 사람수. 

char pidstr[8] = {0,};
// pid를 문자열로 저장한 것.

void receive_print_message(int mtype);
void send_message(int mtype, char * message);

void make_mynickname();
void send_nickname_receive_jobnum();

void wait_message_read_user_data();

void receive_message_wait_passing_night();

void day_discussion();
void receive_message_day_discussion();
void receive_message_vote_someone();

void end_handler(int signo);

int input_number_condition_check(); 



void receive_print_message(int mtype){
	int len = msgrcv(msgid, &mesg, 800, mtype, 0);
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

void make_mynickname(){
	printf("안녕하세요 마피아 게임입니다.\n");
	printf("당신은 게임의 참여자로 선정되었습니다.\n");
	printf("제작 자 : 강대훈, 이동석.\n");
	printf("당신의 닉네임을 정해주세요!\n");
	printf("입력(특수문자 제외 20자이내) : ");

	my_data = malloc(sizeof(char) * 30);
	scanf("%s", my_data);
	//printf("네, 당신의 nickname은 %s입니다!\n", my_data);
	
	strcpy(my_nickname, my_data);
	mypid = (int)getpid();
	sprintf(pidstr, "%d", mypid);
	//printf("그리고 당신의 pid은 %s입니다!\n", pidstr);

	strcat(my_data, "-");
	strcat(my_data, pidstr);
	//printf("그리고 당신의 메세지는 %s입니다!\n", my_data);

	// 닉네임을 입력 받고, 자신의 pid와 합쳐서 메세지를 만드는 과정.
}

void send_nickname_receive_jobnum(){
	key = ftok("mg", 1);
	// key 값을 생성합니다.
	if((msgid = msgget(key, 0)) < 0){
		perror("msgget");
		exit(1);
	}
	// key값과 IPC_CREAT은 새로운 키면 식별자를 새로 생성합니다. 그렇지 않으면 새로 생성하지 않습니다.
	// 0644 는 펄미션입니다. 110 100 100
	// 이것은  .. 그렇습니다.
	// 반환하는 것은 message identifier 메세지 식별자 입니다.

	send_message(1, my_data);
	receive_print_message(2);
	// 만들어둔 닉네임과 pid를 보냅니다.

	mymtype = atoi(mesg.mtext);
	//printf("너의 mymtype : %d \n", mymtype);
	// 클라이언트가 쓸 mtype을 저장.

	receive_print_message(mymtype);
	myjobnum = atoi(mesg.mtext);
	//printf("너의 myjobnum : %d \n", myjobnum);
	//참여자의 직업 번호를 저장.

	if(myjobnum == Mafia){
		receive_print_message(mymtype);
		// 마피아일 경우에는 마피아 수를 추가적으로 받음.
		nopb_jobs[Mafia] = atoi(mesg.mtext);
		printf("현재 마피아 수 : %d\n", nopb_jobs[Mafia]);
		// 여기 뒤에 마피아가 2명 이상일 때, 서버역할인지 클라이언트
		//// 1번째 정보.

		if(nopb_jobs[Mafia] >=2){
			receive_print_message(mymtype);
			mafia_flag = atoi(mesg.mtext);
			// 2번째는 마피아 클라인가 서버인가?

			receive_print_message(mymtype);
			// 3번째는 마피아 정보.
		}
		

	

	}
}

void print_your_data_read_user_data(){

	printf("\n\n--------------------------------\n");
	printf("당신이 현재 알고 있는 정보들.\n");
	printf("--------------------------------\n");
	printf("당신의 참여자 번호는 %d입니다.\n", mymtype -3);
	printf("당신의 닉네임은 %s입니다.\n", my_nickname);
	printf("당신의 pid는 %d입니다.\n", mypid);
	printf("당신의 직업은 %s입니다.\n", nameofjobs[myjobnum]);

	if ((rfp = fopen("user_data.txt", "r")) == NULL) {
        perror("fopen: user_data.txt");
        exit(1);
    }
    while ((n = fread(buf, sizeof(char), 1024, rfp)) > 0) {
        //fwrite(buf, sizeof(char), n, 1);
        printf("%s", buf);
    }
    fclose(rfp);
    printf("\n");
    // 유저 정보를 읽어 출력합니다.
}

void wait_message_read_user_data(){
	receive_print_message(mymtype);
	// 메세지를 받습니다.
	print_your_data_read_user_data();
	
}

int input_number_condition_check_2(char input [30]){
	//printf("%d\n", strlen(input));
  if(strlen(input) !=2){
    return -1;
  }
  if ( '0' <= input[0] && input[0] <= '9')
  {
    int num = input[0] - '0';
    //printf("성공 \n");
    return num;
  }
  else
  	return -1;
}

void socket_client(){
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
          perror("socket");
          exit(1);
      }
  
      memset((char *)&sin, '\0', sizeof(sin));
      sin.sin_family = AF_INET;
      sin.sin_port = htons(PORTNUM);
      sin.sin_addr.s_addr = inet_addr("127.0.0.1");
 
      if (connect(sd, (struct sockaddr *)&sin, sizeof(sin))) { // 서버에 연결 요청함.
          perror("connect");
          exit(1);
      }
     while(1){
      printf("메세지를 기다립니다...\n");
     	if (recv(sd, s_message, sizeof(s_message), 0) == -1) {
          perror("recv");
          exit(1);
      		}
      	printf("From Server : %s\n", s_message);
        if(input_number_condition_check_2(s_message) >= 0){
          break;
        }
      	printf("할말을 입력하세요.(0이상 9이하의 한자리 숫자를 받으면 종료됩니다.): ");
        fgets(s_message, sizeof(s_message), stdin);

        if (send(sd, s_message, strlen(s_message) + 1, 0) == -1) {
          perror("send");
          exit(1);
       }
        if(input_number_condition_check_2(s_message) >= 0){
          break;
        }
     }
     printf("합의된 죽일 사람 번호 : %d\n", atoi(s_message));
     //test 
     close(sd);

}
void socket_server(){
	
	      if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {  // 인터넷 소켓 생성.
          perror("socket");
          exit(1);
      }
  
      memset((char *)&sin, '\0', sizeof(sin)); // 
      sin.sin_family = AF_INET;
      sin.sin_port = htons(PORTNUM);
      sin.sin_addr.s_addr = inet_addr("127.0.0.1");
  
      if (bind(sd, (struct sockaddr *)&sin, sizeof(sin))) {
          perror("bind");
          exit(1);
      }
      if (listen(sd, 5)) { // 클라이언트의 연결을 기다림 (최대 5개)
          perror("listen");
          exit(1);
      }
  
      
      if ((ns = accept(sd, (struct sockaddr *)&cli, &clientlen))==-1) { // 클라이언트의 접속을 수용함.
          perror("accept");
          exit(1);
      }
      printf("connected\n");
      //sprintf(buf, "Your IP address is %s", inet_ntoa(cli.sin_addr));
      
      while(1){
        printf("할말을 입력하세요.(0이상 9이하의 한자리 숫자를 받으면 종료됩니다.): ");
        fgets(s_message, sizeof(s_message), stdin);
        //scanf("%s",s_message);

        if (send(ns, s_message, strlen(s_message) + 1, 0) == -1) {
          perror("send");
          exit(1);
       }
       if(input_number_condition_check_2(s_message) >= 0){
          break;
        }
        printf("메세지를 기다립니다...\n");
       if (recv(ns, s_message, sizeof(s_message), 0) == -1) {
          perror("recv");
          exit(1);
          }
          printf("From Client : %s\n", s_message);
          if(input_number_condition_check_2(s_message) >= 0){
          break;
        }
      }
      printf("합의된 죽일 사람 번호 : %d\n", atoi(s_message));
     
  
      close(ns);
      close(sd);


}


void receive_message_wait_passing_night(){
	receive_print_message(mymtype);
	// clear?
	print_your_data_read_user_data();
	if(myjobnum == Mafia){
		if(nopb_jobs[Mafia] ==1){
			printf("마피아를 제외하고 죽일 상대의 num을 입력하세요.\n");
			killnum = input_number_condition_check();
			memset(mesg.mtext, 0, sizeof(mesg.mtext));
			sprintf(mesg.mtext, "%d", killnum);
			send_message(1, mesg.mtext);
		}
		else if(nopb_jobs[Mafia] ==2){
			if(mafia_flag == 0){
				sleep(1);
				socket_client();
			}
			else if( mafia_flag ==1){
				socket_server();
			}
			send_message(1,s_message);
		}
	}
	//마피아만 죽일 상대를 정해 사회자에게 알려준다.
	// 여기서 2명 이상일 때 소켓통신 해야할듯.
	// 다른 참여자는 낮이 되길 기다린다.건

	if(myjobnum ==Mafia){
		receive_print_message(mymtype);
		nopb_jobs[Mafia] =atoi(mesg.mtext);		
		// 마피아는 추가적으로 메세지를 한번 더 받는다.
	}
}

int input_number_condition_check(){
	char input [30];
	while (1) {
		printf("입력 : ");
		scanf("%s", input);

		if(strlen(input) !=1){
			printf("조건에 맞지 않습니다. 다시 입력하세요. \n");
			continue;
		}
		if (strcmp(input, "0")==0)			
		{
			//printf("성공 \n");
			return 0;
		}
		else if (atoi(input) > 0 && atoi(input) <= 9)
		{
			int num = atoi(input);
			//printf("성공 \n");
			return num;
		}
		else
		{
			printf("조건에 맞지 않습니다. 다시 입력하세요. \n");
			continue;
		}
	}
}

void day_discussion(){
	key_t sh_key = ftok("shmfile", 1);
	int shmid = shmget(key, MEMORY_SIZE, 0);
	char * shamddr = (char*)shmat(shmid,(char *)NULL, 0); // shared memory 연결 
	while(1)
        {
        	if(discuss_end_flag ==1){
        		printf("토론이 종료되었습니다.\n");
        		printf("잠시 기다려주세요.\n");
        		discuss_end_flag = 0;
        		break;
        	}
            int a;
            printf("어떤 행동 할래?\n");
            printf("1 : 메세지 날리기.\n");
            printf("2 : 메세지 보기.\n");

			scanf("%d", &a);
			getchar();
		
			if (a==1)
			{	
				
				char str1[1000] = {0,};
				printf("메세지를 입력하세요.\n입력 : ");
				//scanf("%s", str1);
				fgets(str1,sizeof(str1), stdin); 
				////fgets(buf, sizeof(buf), stdin);
				
				strcat(shamddr, my_nickname);
				shamddr[strlen(shamddr)] = ':';
				strcat(shamddr, str1);
				shamddr[strlen(shamddr)] = '\n';
			//	shmdt((char *)shmaddr); // shared memory 연결 해제
			}
			else if(a==2)
			{
				printf("===================================\n");
				printf("%s", shamddr); // shared memory에 있는 내용을 출력
				printf("===================================\n");

			}
	 }
	 // 공유 메모리를 이요함.
	 // 입력이 1이면 메세지를 입력.
	 // 입력이 2면 지금 까지 있는 메세지를 보여줌.


	 shmdt(shamddr);

	 send_message(1, "I'm done\n");
	 // 정해진 시간이 끝났을 떄 나 끝났다는 메세지를 보냄.
}

void receive_message_day_discussion(){
	receive_print_message(mymtype);
	// clear?
	print_your_data_read_user_data();
	day_discussion();
}

void end_handler(int signo){
	if(signo == SIGUSR2){
		printf("시민은 승리하였습니다!\n");
	}

	if(signo == SIGUSR1){
		printf("마피아가 승리하였습니다!\n");
	}

	if(signo == SIGQUIT){
		printf("당신은 죽임을 당했습니다. \n");
		printf("---GameOver..---\n");

	}


	if(signo == SIGALRM){
		discuss_end_flag = 1;
	}
	
	if(signo ==SIGUSR1 || signo == SIGUSR2){
		if ((rfp = fopen("user_data.txt", "r")) == NULL) {
        	perror("fopen: user_data.txt");
        	exit(1);
    	}

	    while ((n = fread(buf, sizeof(char), 1024, rfp)) > 0) {
	        //fwrite(buf, sizeof(char), n, 1);
	        printf("%s", buf);
	    }
	    fclose(rfp);
	    printf("\n");
	}


	if(signo == SIGUSR1 && signo ==SIGUSR2 && signo ==SIGQUIT){
	  close(ns);
      close(sd);
      // 종료 할때, 소켓을 닫아주자.
	}
    // 결과 공개.
	if(signo != SIGALRM){
    	exit(1);
	}
}

void receive_message_vote_someone(){
	receive_print_message(mymtype);
	// clear?
	print_your_data_read_user_data();
	
	printf("특정 유저의 번호를 지목해주세요. \n");
	memset(mesg.mtext, 0, sizeof(mesg.mtext));
	int selectnum = input_number_condition_check();
	sprintf(mesg.mtext, "%d", selectnum);
	send_message(1, mesg.mtext);
	// 특정 유저의 번호를 입력 받아 그대로 사회자에게 전달.
}

int main(void) {


	act.sa_flags = 0; // 아무것도 설정안한 것.
    act.sa_handler = end_handler; //핸들러로 위에 것을 지정한것.
    if (sigaction(SIGUSR2, &act, (struct sigaction *)NULL) < 0) {
        perror("sigaction");
        exit(1);
    }
    if (sigaction(SIGUSR1, &act, (struct sigaction *)NULL) < 0) {
        perror("sigaction");
        exit(1);
    }
    if (sigaction(SIGQUIT, &act, (struct sigaction *)NULL) < 0) {
        perror("sigaction");
        exit(1);
    }
    if (sigaction(SIGALRM, &act, (struct sigaction *)NULL) < 0) {
        perror("sigaction");
        exit(1);
    }
	//signal 설정
	//client 프로그램 시작.

// 	1. 초기설정.
// 1-1. 자신의 닉네임 정하기 닉네임 보내고 직업정보 받기.
// 1-2. 메시지 기다리고 유저 정보 확인하기

	make_mynickname();
	send_nickname_receive_jobnum();
// 1-1. 자신의 닉네임 정하기 닉네임 보내고 직업정보 받기.


	wait_message_read_user_data();
// 1-2. 메시지 기다리고 유저 정보 확인하기
	while (1){
// (밤)
// 2. 메세지 받고 밤이 지나길 기다리기
		receive_message_wait_passing_night();

// (낮)
// 3. 메세지를 받고 낮에 토론하기
		receive_message_day_discussion();


//4.투표하고 투표결과 받기.
		receive_message_vote_someone();
		receive_print_message(mymtype);
		if(myjobnum ==Mafia){
			receive_print_message(mymtype);
			nopb_jobs[Mafia] =atoi(mesg.mtext);		
			// 마피아는 추가적으로 메세지를 한번 더 받는다.
		}
// 투표 결과를 받음.

	}
//5. 2-4를 반복하기.
	

	return 0;
}

