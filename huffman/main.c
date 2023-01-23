#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _node{
	char alpha;
	long long freq;
	struct _node* lchild;
	struct _node* rchild;
} node;
long long freq[128] = {0,}; //빈도수를 저장하는 배열
char s[128]; //makeTable에서 허프만 코드를 저장할 문자열
int s_ind = 0;
void huffman_encode(char* inputf);
void huffman_decode(char* inputf);
void makeTable(node* cur, char* str, char* htable[]);
int main(int argc, char* argv[]) {
	
	if(!strcmp(argv[1], "-c")){
		huffman_encode(argv[2]);
	}
	else if(!strcmp(argv[1], "-d")) {
		huffman_decode(argv[2]);
	}
}

node* makeNode(char al, int f, node* lc, node* rc){
	node* n = (node*)malloc(sizeof(node));
	n->alpha = al; n->freq = f; n->rchild = rc; n->lchild = lc; 
	return n;
}
void huffman_encode(char* inputf){
	FILE* fip = fopen(inputf, "r");
	char readBuf[100];
	int numOfSym = 0;
	node* tree[128]; for(int i=0; i<128; i++) tree[i] = NULL;
	int check[128]; for(int i=0; i<128; i++) check[i] = -1;
	 //tree의 i번 노드가  이미 합쳐져 더이상 유효하지 않는 등 노드가 없는 상태를 -1, 있는 상태를 1
	while(fgets(readBuf, 100, fip)){ //빈도수를 저장
		int blen = strlen(readBuf);
		for(int i=0; i<blen; i++){
			freq[(int)readBuf[i]]++;
		}
	}
	int ind = 0;
	for(int i=0; i<128; i++){
		if(freq[i]>0){
			numOfSym++;
			tree[ind] = makeNode((char)i, freq[i], NULL, NULL); //인풋파일에 출현하는 각 문자들을 트리 구조에 빈도수와 함께 저장
			check[ind++] = 1;
		}
	}
	int min, min2;
	//트리구조 만들기
	for(int i=0; i<ind-1; i++){
		int j = 0;  
		while(check[j]==-1) {j++;} //tree에서 합쳐지지 않은 노드 중 첫 번째 원소 인덱스
		min = j; //printf("처음 min %d\n", min); 
		//합쳐지지 않은 노드 중 lowest frequency 갖는 노드 인덱스 찾기
		for(j = min; j<ind; j++){
			if(check[j]==1&&tree[min]->freq>tree[j]->freq)
				{min = j;}
		}
		j = 0;
		while(j<128){if(j==min) {j++; continue;} if(check[j]==1) break; j++;
		}
		min2 = j;
		for(j = 0; j<ind; j++){ //printf("%d의 freq는 %d", j, tree[j]->freq);
                        if(check[j]==1&&tree[min2]->freq>tree[j]->freq&&j!=min)
                                min2 = j;
		}
		//printf("min은 %d 빈도수 %d min2는 %d 빈도수 %d\n", (int)tree[min]->alpha, tree[min]->freq, (int)tree[min2]->alpha, tree[min2]->freq );
		check[min2] = -1; 
		node* n = (node*)malloc(sizeof(node));
	        n->alpha = 0; n->freq = tree[min]->freq+tree[min2]->freq; n->lchild = tree[min]; n->rchild = tree[min2];
		tree[min] = n;
		//tree[min] = makeNode(NULL, tree[min]->freq+tree[min2]->freq, tree[min], tree[min2]);
		//printf("new node의 lchild rchild %c %c이다", tree[min]->lchild->alpha, tree[min]->rchild->alpha);	
		
	}
	//printf("numOfSym %d\n", numOfSym);	
	//트리구조를 통해 table만들기
	char* htable[128]; char str[128];
	memset(htable, 0, sizeof(htable));//허프만 테이블 초기화
	makeTable(tree[min]->lchild, "0", htable);
	makeTable(tree[min]->rchild, "1", htable);
	//for(int i=0; i<128; i++){if(htable[i]!=0) }//printf("char %c ==> %s\n", (char)i, htable[i]);}
	//encode 파일 생성
	char eFilename[100] = "";
	strcat(eFilename, inputf);
	strcat(eFilename, ".zz");
	FILE* fop = fopen(eFilename, "wb");
	//printf("numOfSym %d\n", numOfSym);
	//허프만 트리를 인코드 파일에 저장하기
	fwrite(&numOfSym, sizeof(numOfSym), 1, fop);
	char writeBuf[100];
	for(int i=0; i<128; i++){
		if(htable[i]!=0){ //즉 출현해서 허프만 코드 갖는 문자열이면
			writeBuf[0] = (char)i;
			writeBuf[1] = (char)strlen(htable[i]);
			strcpy(&writeBuf[2], htable[i]);
			fwrite(writeBuf, sizeof(char), 2+strlen(htable[i]), fop);
		}
	}
	//문자열을 허프만 코드로 치환하여 인코드 파일에 저장하기
	fip = fopen(inputf, "r"); 
	int locTotalNumBit = ftell(fop); //현재까지 파일 포인터의 위치를 반환, 이 위치에 허프만 코드 총 길이를 저장할 4바이트의 공간을 남겨둔다.
	fseek(fop, 8, SEEK_CUR); //공간 남겨두고 위치를 이동시킨다.
	int flag = 0;
	char buffer[100]; 
	char bitBuf[100]; //생성되는 비트스트림을 저장하는 버퍼
	memset(bitBuf, 0, 100);
	int bitBufInd = 0; //현재 비트스트림을 저장할 버퍼
	int bitShiftCnt = 7; //비트쉬프트 횟수
	long long totalBitNum = 0;

        while(fgets(buffer, 100, fip)){
                int len = strlen(buffer);
		for(int i=0; i<len; i++){
			char* huffmanCode = htable[(int)buffer[i]]; //문자의 허프만 코드
			//printf("%s ", huffmanCode);
			for(int j=0; j<strlen(huffmanCode); j++){ 
				//허프만 코드를 비트버퍼에 저장하는 과정
				char val = 0; 
				if(huffmanCode[j]=='0') val = 0;
				else val = 1;
				val = val<<bitShiftCnt; bitShiftCnt--;
				bitBuf[bitBufInd]|= val; flag = 1;
				totalBitNum++;
				if(bitShiftCnt<0){ //1바이트가 다 찼다면 다시 비운다.
					bitShiftCnt = 7; 
					bitBufInd++;
					if(bitBufInd>=100){//버퍼가 다 찼다면 인코드 파일에 내용을 쓴 뒤 다시 채울 준비를 한다.
						fwrite(bitBuf, 1, 100, fop); flag = 0;
						memset(bitBuf, 0, 100);
						bitBufInd = 0;
					}
				}
			}
		}
        }
	//printf("Total bit num %lld\n", totalBitNum);
	//bitBuf가 꽉차지 않은 채로 종료 되었다면
	if(flag==1){ //남아있는 내용들을 마저 적어준다.
		fwrite(bitBuf, 1, bitBufInd+1, fop);
	} 
	//맨 처음 위치를 저장해놨던 곳으로 가서 비트 수를 기록한다.
	fseek(fop, locTotalNumBit, SEEK_SET);
	fwrite(&totalBitNum, sizeof(long long), 1, fop);

	fclose(fop);	
}

void makeTable(node* cur, char *str, char* htable[]){
	if(!cur->lchild&&!cur->rchild){ //leaf노드에 도달한 경우
		//printf("\n%c %s", cur->alpha, str);
		char* huf = (char*)malloc(strlen(str));
		strncpy(huf, str, strlen(str)); 
		htable[(int)cur->alpha] = huf; //허프만 테이블에 코드를 저장해주고
	}
	else{	char* ls = (char*)malloc(strlen(str)+1);
		strncpy(ls, str, strlen(str));	strcat(ls, "0");
		char* rs = (char*)malloc(strlen(str)+1);
                strncpy(rs, str, strlen(str)); strcat(rs, "1");	
		makeTable(cur->lchild, ls, htable);
		makeTable(cur->rchild, rs, htable);
	}

}

void huffman_decode(char* inputf){
	FILE* fip = fopen(inputf, "rb"); //인코드 파일 읽기 모드
	//허프만 트리 재구성
	int numOfSym;
	fread(&numOfSym, sizeof(int), 1, fip);
	node* root = (node*)malloc(sizeof(node));
        root->lchild = NULL; root->rchild = NULL;
        node* cur = root;
	//printf("Num of sym %d\n", numOfSym);
	for(int i=0; i<numOfSym; i++){
		char symAndLen[2]; //심볼과 심볼의 코드 길이 저장
		fread(symAndLen, 2, 1, fip);
		char buf[100];
		fread(buf, 1, (int)symAndLen[1], fip); //허프만 코드를 buf에 저장
		buf[(int)symAndLen[1]] = 0;
		//printf("%c %d==>%s\n", symAndLen[0], (int)symAndLen[1], buf);
		//트리 구성 시작
		cur = root; 
		for(int j=0; j<(int)symAndLen[1]; j++){
			if(buf[j]=='0'){
				if(cur->lchild==NULL){//부모 노드가 될 cur을 찾음
					cur->lchild = (node*)malloc(sizeof(node));
					cur->lchild->lchild = NULL;
					cur->lchild->rchild = NULL;	
				}
				cur = cur->lchild;
			}
			else{
				if(cur->rchild==NULL){//부모 노드가 될 cur을 찾음
                                        cur->rchild = (node*)malloc(sizeof(node));
                                        cur->rchild->lchild = NULL;
                                        cur->rchild->rchild = NULL;
                                }
                                cur = cur->rchild;
			}
		}
		//cur = (node*)malloc(sizeof(node));
		cur->alpha = symAndLen[0];
	}
	//트리 형성 완료
	//디코딩 수행
	char eFilename[100] = "";
	strcat(eFilename, inputf);
        strcat(eFilename, ".yy");
        FILE* fop = fopen(eFilename, "wb");	
	long long numBitRead = 0;
	fread(&numBitRead, sizeof(long long), 1, fip); //총 비트 수 저장 
	//printf("Total Num of Bit %d\n", numBitRead);
	cur = root;
	char readBuf[100];
	while(1){
		int size = fread(readBuf, 1, 100, fip); //1bit짜리를 100개 만큼 readBuf에 저장
		//printf("readBuf %s\n", readBuf);
		if(size==0) break;
		for(int i=0; i<size; i++){
			for(int j=0; j<8; j++){//8개의 비트
				if((char)(readBuf[i] & 0x80)==0){
					cur = cur->lchild;
				}
				else cur = cur->rchild;
				readBuf[i] = readBuf[i]<<1;
				numBitRead--;
				if(cur->lchild==NULL&&cur->rchild==NULL){
					fprintf(fop, "%c", cur->alpha);
					//printf("%c", cur->alpha);
					cur = root;
				}
				if(numBitRead==0) {fclose(fip); return;}
			}
		}
	}
	
}
 
