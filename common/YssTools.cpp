#include "YssTools.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <arpa/inet.h>
#include<signal.h>
#include<sys/wait.h>
#include "YssCommon.h"
#include "YsWordSegmentLR.h"

YS_FileProcessor::YS_FileProcessor(const char* filename): m_strFileName(filename), m_filePtr(NULL),m_lFileLen(-1)
{
	
}

YS_FileProcessor::~YS_FileProcessor()
{
	CloseFile(); 
}
	
bool YS_FileProcessor::OpenForRead()
{
	m_filePtr = fopen(m_strFileName.c_str(), "r");
	if(m_filePtr == NULL)
		return false;
	SetFileLen();
	return true;
}

bool YS_FileProcessor::OpenForWrite()
{
	m_filePtr = fopen(m_strFileName.c_str(), "w");
	if(m_filePtr == NULL)
		return false;
	return true;
}

bool YS_FileProcessor::OpenForAppend()
{
	m_filePtr = fopen(m_strFileName.c_str(), "a");
	if(m_filePtr == NULL)
		return false;
	return true;
}
	
bool YS_FileProcessor::CloseFile()
{
	if(m_filePtr != NULL)
	{
		fclose(m_filePtr);
		m_filePtr = NULL;
	}
	return true;
}
	
bool YS_FileProcessor::SeekByIndex(long index)
{
	if( fseek(m_filePtr, index, SEEK_SET) < 0 )
		return false;
	return true;
}

int YS_FileProcessor::GetIntFromJavaToC(int data)
{
	long iRetStat = 0;
	char* cPtr = (char*)&iRetStat;
	cPtr[3] = (char) (data & 0xff);//javaλ��5  ��ӦC++ λ��4
	cPtr[2] = (char) (data >> 8 & 0xff);//javaλ��6  ��ӦC++ λ��3
	cPtr[1] = (char) (data >> 16 & 0xff);//javaλ��7  ��ӦC++ λ��2
	cPtr[0] = (char) (data >> 24 & 0xff);//javaλ��8  ��ӦC++ λ��1
	
	return iRetStat;
}

int YS_FileProcessor::ReadInt()
{
	int iRetValue = 0;
	int iRetStat = fread((void*)&iRetValue, JAVA_INT_BIT, ONE_TIME, m_filePtr);
	if( iRetStat != ONE_TIME)
		return -1;
	return GetIntFromJavaToC(iRetValue);
}

long YS_FileProcessor::GetLongFromJavaToC(long data)
{
	long iRetStat = 0;
	char* cPtr = (char*)&iRetStat;
	cPtr[7] = (char) (data & 0xff);      //javaλ��1  ��ӦC++ λ��8
	cPtr[6] = (char) (data >> 8 & 0xff); //javaλ��2  ��ӦC++ λ��7
	cPtr[5] = (char) (data >> 16 & 0xff);//javaλ��3  ��ӦC++ λ��6
	cPtr[4] = (char) (data >> 24 & 0xff);//javaλ��4  ��ӦC++ λ��5
	cPtr[3] = (char) (data >> 32 & 0xff);//javaλ��5  ��ӦC++ λ��4
	cPtr[2] = (char) (data >> 40 & 0xff);//javaλ��6  ��ӦC++ λ��3
	cPtr[1] = (char) (data >> 48 & 0xff);//javaλ��7  ��ӦC++ λ��2
	cPtr[0] = (char) (data >> 56 & 0xff);//javaλ��8  ��ӦC++ λ��1
	
	return iRetStat;
}

long YS_FileProcessor::ReadLong()
{
	long iRetValue;
	int iRetStat = fread((void*)&iRetValue, JAVA_LONG_BIT, ONE_TIME, m_filePtr);
	if( iRetStat != ONE_TIME)
		return -1;
	return GetLongFromJavaToC(iRetValue);
}

bool YS_FileProcessor::ReadData(void* szBuf, long size) {
	long iLeft = size;
	long iReadNum;
	long iRet;

	memset(szBuf, 0x00, size);
	char* bufBegin = (char*)szBuf;

	while (iLeft > 0) {
		
		iReadNum = iLeft > 2048 ? 2048 : iLeft;
		iRet = fread((char*)bufBegin, 1, iReadNum, m_filePtr);
		bufBegin += iReadNum;
		if (iRet != iReadNum) {
			if (!feof(m_filePtr))
				return false;
			return true;
		}
		iLeft -= iRet;
	}
	return true;
}

bool YS_FileProcessor::ReadLine(char* szBuf, long uiBufLen) {
    szBuf[0] = '\0';
	if (fgets(szBuf, uiBufLen, m_filePtr) == NULL) {
		if (!feof(m_filePtr))
			return false;
		return true;
	}
	if (szBuf[strlen(szBuf) - 1] == ' ' || szBuf[strlen(szBuf) - 1]== '\n')
		szBuf[strlen(szBuf) - 1] = 0x00;
	return true;
}

bool YS_FileProcessor::ReadByteArray(void* dest, long size)
{
	return ONE_TIME==fread(dest, size, ONE_TIME, m_filePtr);	
}

bool YS_FileProcessor::ReadCharArray(char* dest, long size)
{
	return size==fread(dest, sizeof(char), size, m_filePtr);
}

bool YS_FileProcessor::WriteLine(const char* buff)
{
	int ret0 = strlen(buff);
	int ret1 = fprintf(m_filePtr,"%s\n",  buff);
	return ret0+1 == ret1;
}

bool YS_FileProcessor::WriteBin(const void* buff, long size)
{
	return ONE_TIME==fwrite(buff, size, ONE_TIME, m_filePtr);
}

long YS_FileProcessor::SetFileLen()
{
	struct stat st;
	if(fstat(fileno(m_filePtr), &st) < 0 )
		return -1;
	m_lFileLen = st.st_size;
	return 0;	
}

bool YS_FileProcessor::IsEnd()
{
	if(feof(m_filePtr))
		return true;
	return false;
}

int YS_FileProcessor::GetFileFd()
{
	return fileno(m_filePtr);
}

/*Ys_BadCharacter info */
Ys_BadCharacter* Ys_BadCharacter::m_instance = NULL;
string Ys_BadCharacter::m_strBadChar(",.~!@#$%^&*)(_+-=/*;':\"?><,./\\������������������������������������������");
string Ys_BadCharacter::m_strBadChineseChar(",$.$~$!$@$#$%$^$&$*$)$($_$+$-$=$/$*$;$'$:$\"$?$>$<$/$\\$��$��$��$��$����$��$��$��$��$��$��$��$��$��$��$��$��$��$��$��$--$?$��$��$��$����$able$about$above$according$accordingly$across$actually$after$afterwards$again$against$all$allow$allows$almost$alone$along$already$also$although$always$am$among$amongst$an$and$another$any$anybody$anyhow$anyone$anything$anyway$anyways$anywhere$apart$appear$appreciate$appropriate$are$aren't$around$as$a's$aside$ask$asking$associated$at$available$away$awfully$be$became$because$become$becomes$becoming$been$before$beforehand$behind$being$believe$below$beside$besides$best$better$between$beyond$both$brief$but$by$came$can$cannot$cant$can't$cause$causes$certain$certainly$changes$clearly$c'mon$co$com$come$comes$concerning$consequently$consider$considering$contain$containing$contains$corresponding$could$couldn't$course$c's$currently$definitely$described$despite$did$didn't$different$do$does$doesn't$doing$done$don't$down$downwards$during$each$edu$eg$eight$either$else$elsewhere$enough$entirely$especially$et$etc$even$ever$every$everybody$everyone$everything$everywhere$ex$exactly$example$except$far$few$fifth$first$five$followed$following$follows$for$former$formerly$forth$four$from$further$furthermore$get$gets$getting$given$gives$go$goes$going$gone$got$gotten$greetings$had$hadn't$happens$hardly$has$hasn't$have$haven't$having$he$hello$help$hence$her$here$hereafter$hereby$herein$here's$hereupon$hers$herself$he's$hi$him$himself$his$hither$hopefully$how$howbeit$however$i'd$ie$if$ignored$i'll$i'm$immediate$in$inasmuch$inc$indeed$indicate$indicated$indicates$inner$insofar$instead$into$inward$is$isn't$it$it'd$it'll$its$it's$itself$i've$just$keep$keeps$kept$know$known$knows$last$lately$later$latter$latterly$least$less$lest$let$let's$like$liked$likely$little$look$looking$looks$ltd$mainly$many$may$maybe$me$mean$meanwhile$merely$might$more$moreover$most$mostly$much$must$my$myself$name$namely$nd$near$nearly$necessary$need$needs$neither$never$nevertheless$new$next$nine$no$nobody$non$none$noone$nor$normally$not$nothing$novel$now$nowhere$obviously$of$off$often$oh$ok$okay$old$on$once$one$ones$only$onto$or$other$others$otherwise$ought$our$ours$ourselves$out$outside$over$overall$own$particular$particularly$per$perhaps$placed$please$plus$possible$presumably$probably$provides$que$quite$qv$rather$rd$re$really$reasonably$regarding$regardless$regards$relatively$respectively$right$said$same$saw$say$saying$says$second$secondly$see$seeing$seem$seemed$seeming$seems$seen$self$selves$sensible$sent$serious$seriously$seven$several$shall$she$should$shouldn't$since$six$so$some$somebody$somehow$someone$something$sometime$sometimes$somewhat$somewhere$soon$sorry$specified$specify$specifying$still$sub$such$sup$sure$take$taken$tell$tends$th$than$thank$thanks$thanx$that$thats$that's$the$their$theirs$them$themselves$then$thence$there$thereafter$thereby$therefore$therein$theres$there's$thereupon$these$they$they'd$they'll$they're$they've$think$third$this$thorough$thoroughly$those$though$three$through$throughout$thru$thus$to$together$too$took$toward$towards$tried$tries$truly$try$trying$t's$twice$two$un$under$unfortunately$unless$unlikely$until$unto$up$upon$us$use$used$useful$uses$using$usually$value$various$very$via$viz$vs$want$wants$was$wasn't$way$we$we'd$welcome$well$we'll$went$were$we're$weren't$we've$what$whatever$what's$when$whence$whenever$where$whereafter$whereas$whereby$wherein$where's$whereupon$wherever$whether$which$while$whither$who$whoever$whole$whom$who's$whose$why$will$willing$wish$with$within$without$wonder$won't$would$wouldn't$yes$yet$you$you'd$you'll$your$you're$yours$yourself$yourselves$you've$zero$zt$ZT$zz$ZZ$һ$һ��$һЩ$һ��$һ��$һ��$һ��$һ����$һ��$һʱ$һ��$һ��$һ��$һƬ$һֱ$һ��$һ��$һ��$һ��$һ��$��һ$����$����$��ȥ$����$����$����$����$��ȥ$����$����$��һ$����$����$����$����$����$����$����$��ֻ$����$��ͬ$����$����$����$����$��Ω$����$����$����$����$����$����$��Ȼ$����$����$����$����$��Ҫ$����$����$����$����$��$����$���$���ͬʱ$ר��$��$����$�ϸ�$����$��$����$����$��С$�м�$�ḻ$��$Ϊ$Ϊ��$Ϊ��$Ϊʲô$Ϊʲ��$Ϊ��$Ϊ��$����$��Ҫ$����$��$����$ô$֮$֮һ$֮ǰ$֮��$֮��$֮����$֮��$�ں�$��$��$Ҳ$Ҳ��$Ҳ��$Ҳ��$��$�˽�$��ȡ$��$����$���Ǻ�$����$����$����$����$�˼�$ʲô$ʲô��$ʲ��$���$����$����$����$��Ȼ$��$����$�Ӷ�$��$����$����$����$����$��$����$����$��Ϊ$�Ա�$����$��ǰ$�Լ�$�Ժ�$����$����$����$����$������$����$��$��$�κ�$��ƾ$����$��ͼ$ΰ��$�ƺ�$�Ƶ�$��$����$��$�ο�$�δ�$��ʱ$��Ϊ$��$����$���$ʹ��$ʹ��$����$��$����$����$�ٽ�$����$��$����$��$��ʹ$�Ȼ�$��Ȼ$����$��ʹ$����$����$����$��$����$���$�Ⱥ�$����$����$ȫ��$ȫ��$��$��ͬ$����$��$��һ$����$���$����$����$����$��ʵ$���$����$�����˵$����˵��$����$����$��˵$ð$��$����$����$׼��$��$����$��ʱ$ƾ$ƾ��$��ȥ$����$����$�ֱ�$��$��$���$��˵$��$ǰ��$ǰ��$ǰ��$ǰ��$��֮$����$����$��ǿ$ʮ��$��$����$��ʹ$����$����$����$ȴ��$ԭ��$��$��$����$��ʱ$����$˫��$��֮$��Ӧ$��ӳ$������$������˵$ȡ��$�ܵ�$���$��$��һ����$����$ֻ��$ֻ��$ֻҪ$ֻ��$��$����$�ٿ�$����$��$����$����$����$�ɼ�$��$����$����$��λ$����$����$����$����$����$ͬ$ͬһ$ͬʱ$ͬ��$����$����$��$����$��$��$����$��$����$֨$ѽ$��$Ż$��$��$�غ�$��$��Χ$��$��$����$զ$��$��$��$��$����$��$��$��$����$��$��$��ѽ$��Ӵ$��$Ӵ$Ŷ$��$��$�ĸ�$��Щ$�Ķ�$����$����$����$����$�ı�$����$��$���$��$��$��$ɶ$��$ž��$ι$��$��$����$��$��$��$��$�µ�$��$��$��$��$��$��Ϊ$���$���$��Ȼ$��$����$��$���$���$����$����$����$��$����$����$���$����$�����$���$���$����$��Լ$����$ʧȥ$��$����$����$�õ�$����$��$��������$����$���$����$���$���$����$����$��$����$��Ը$����$��$����$���ǵ�$����$��ȫ$��ȫ$���$ʵ��$ʵ��$����$����$����$��$����$��Ӧ$��$����$����$����$����$��$����$����˵$��$����$����$��$����$�޴�$����$��$�Ѿ�$����$����$��$����$������$����$��û��$���$�㷺$Ӧ��$Ӧ��$Ӧ��$����$��ʼ$��չ$����$ǿ��$ǿ��$��$��$��ǰ$��ʱ$��Ȼ$����$�γ�$����$��$�˴�$��$����$��$����$����$��$�ó�$�õ�$����$��Ȼ$��Ҫ$����$��$��ô$��ô��$��ô��$����$����$��֮$����$�ܵ�����$�ܵ���˵$�ܵ�˵��$�ܽ�$�ܶ���֮$ǡǡ�෴$��$��˼$Ը��$��˵$��Ϊ$��$����$�ҵ�$��$����$����$ս��$��$����$����$��ν$��$����$��$�ֻ�$��$��$����$���仰˵$����֮$��$����$����$����$��$�ʴ�$����$����$����$����$����$�޷�$����$��$����$��Ȼ$ʱ��$����$��ȷ$��$�Ƿ�$�ǵ�$��Ȼ$����$��ͨ$�ձ�$����$����$��$���$���$���$����$���$���$��$��Щ$�й�$����$����$����$��Ч$��ʱ$�е�$�е�$����$����$��$��$����$��$����$��$����$����$����$��Ȼ$����$ĳ$ĳ��$ĳЩ$����$����$��ӭ$����$����$����$��$����$��ʱ$�˼�$����$ÿ$ÿ��$ÿ��$ÿ��$ÿ��$��$����$�ȷ�$�Ƚ�$����$û��$��$����$ע��$����$���$����$��˵$��$Ȼ��$Ȼ��$Ȼ��$Ȼ��$��$����$�ر���$����$�ص�$�ִ�$����$��ô$����$����$��$��$����$�ɴ˿ɼ�$��$�Ļ�$Ŀǰ$ֱ��$ֱ��$����$����$�෴$��ͬ$���$��Զ���$��Ӧ$�൱$���$ʡ��$����$����$����$����$����$����$����$��$����$��$֪��$ȷ��$��$����$�ƶ�$ͻ��$ͻȻ$����$��$��$�ȵ�$��$������$��$����$��ʹ$��Ȼ$��ϰ$���$��$����$����$���$���$��$����$����$�̶�$ά��$��������$����$����$��$��$����$����$����$����$����$����$��ϵ$��$�ܷ�$�ܹ�$��$��$�Ը���$�Դ�$�Ը���$�Լ�$�Լ�$����$��$����$����$��$����$����$��Χ$Ī��$���$��$����$��Ȼ$��˵$��Ϊ$�ж�$����$��ʾ$��$Ҫ$Ҫ��$Ҫ����$Ҫ��Ȼ$Ҫô$Ҫ��$Ҫ��$�涨$����$��Ϊ$����$��ʶ$��$����$��$��ʹ$����$��$˵��$��λ$˭$˭֪$��$��$����$���$��$����$Խ��$��$ת��$ת��$ת��$��$��֮$��$�ﵽ$Ѹ��$��$��ȥ$����$����$����$����$��$���$��ô$��ôЩ$��ô��$��ô���$��Щ$����$���$�����˵$��ʱ$����$���$����$���$����$����$����$����$����$����$��$��ͬ$��Ӧ$�ʵ�$����$��$��$ͨ��$ͨ��$���$����$�⵽$����$��$�Ǹ�$��ô$��ôЩ$��ô��$��Щ$�ǻ��$�Ƕ�$��ʱ$����$�Ǳ�$����$����$����$����$��ȡ$����$�ش�$����$��Ҫ$����$����$��ֹ$��$����$����$��$����$����֮��$����$��$����$����$����$��Ҫ$�ǵ�$�ǳ�$��ͽ$��$˳$˳��$����$����$�ǲ���$˵˵");
Ys_BadCharacter* Ys_BadCharacter::GetInstance()
{
	if( NULL == m_instance)
		m_instance = new Ys_BadCharacter();
	return m_instance;
}

bool Ys_BadCharacter::StartUp()
{
	typedef list<string> StrList;
	typedef StrList::iterator StrListIter;
	m_hashBadCharSet.clear();
	StrList attrList;
	CCommon::split(m_strBadChineseChar.c_str(), attrList, SPLIT_CHAR);
	for(StrListIter it=attrList.begin(); it!=attrList.end(); ++it)
	{
		if(!m_hashBadCharSet.insert(it->c_str()).second);
	}
	return true;
}

bool Ys_BadCharacter::findBadChar(const char* ch)
{
	return findBadChar(string(ch));
}

bool Ys_BadCharacter::findBadChar(const string& item)
{
	if(item.size()< MIN_CHAR_LEN) return true;  //�ַ������ȹ��� ���� ����Ӣ���ַ� 
	
	if(m_hashBadCharSet.find(item) != m_hashBadCharSet.end())
	{
		return true;
	}
	else
		return false;
}

bool Ys_BadCharacter::earseInvalidChar(string& str)
{
	string::size_type iPos = str.find("(");
	if(iPos != string::npos)
		str = str.substr(0, iPos);

	iPos = str.find("��");
	if(iPos != string::npos)
		str = str.substr(0, iPos);
		
	return true;
}

YS_WordSegmentAgent* YS_WordSegmentAgent::m_instance = NULL;
YS_WordSegmentAgent* YS_WordSegmentAgent::GetInstance()
{
	if(m_instance == NULL)
		m_instance = new YS_WordSegmentAgent();
	return m_instance;
}
YS_WordSegmentAgent::YS_WordSegmentAgent()
{
	m_wordSegmentLR = new YS_WordSegmentLR();
};

bool YS_WordSegmentAgent::StartUp()
{
	list<string> flieList;
	if( !CCommon::GetDirFilePath(m_strDicPath, flieList))
	{
		return false;
	}

	if(-1 == m_wordSegmentLR->Init(flieList)){
		return false;;
	}
	return true;
}

YS_WordSegmentAgent::~YS_WordSegmentAgent()
{
	delete m_wordSegmentLR;
}

void YS_WordSegmentAgent::ExtractWordSegment(const char* szKeyword, list<YSS_Word>& lstWord)
{
	m_wordSegmentLR->ExtractWordSegment(szKeyword, lstWord);
}

void YS_WordSegmentAgent::DestroyWordSegment(list<YSS_Word>& lstWord)
{
	m_wordSegmentLR->DestroyWordSegment(lstWord);
}

/**/
//����ת��
YS_CodeConverter::YS_CodeConverter(const char *from_charset,const char *to_charset)
{
	m_codeConvertItem = iconv_open(to_charset,from_charset);
}

YS_CodeConverter::~YS_CodeConverter()
{
	iconv_close(m_codeConvertItem);
}

int YS_CodeConverter::Convert(char *cInbuf,int iInlen,char *cOutbuf,int& iOutlen)
{
	printf(" in YS_CodeConverter::Convert\n");
	char **cPin = &cInbuf;
	char **cPout = &cOutbuf;
	memset(cOutbuf,0, iOutlen);
	
	return iconv(m_codeConvertItem,cPin,(size_t *)&iInlen,cPout,(size_t *)&iOutlen);  //-1 error
}

/*signal info*/
SigFunc* mysignal(int signo, SigFunc* func)
{
	struct sigaction act, oact;

	act.sa_handler = func;
	sigfillset(&act.sa_mask);
	act.sa_flags = 0;
	if ((signo == SIGALRM) || (signo == SIGPIPE))
	{
#ifdef SA_INTERRUPT
		act.sa_flags |= SA_INTERRUPT;
#endif
	}
	else
	{
#ifdef SA_RESTART
		act.sa_flags |= SA_RESTART;
#endif
	}
	if (sigaction(signo, &act, &oact) < 0) return (SIG_ERR);
	return (oact.sa_handler);
}

void onSigUsr2(int)
{
	if (fork() == 0)
	{
		abort();
	}
}

void onSigChld(int)
{
	int status;
	while (waitpid(-1, &status, WNOHANG) > 0);
}

void onSigPipe(int)
{
// ���źŴ��������в���ʹ��printf�Ȳ�������Ŀ⺯��
	const char *prefix1 = "ERROR1 ";
	const char *msg = " received SIGPIPE mysignal.\n";

	write(STDOUT_FILENO, prefix1, strlen(prefix1));
	write(STDOUT_FILENO, msg, strlen(msg));
}
