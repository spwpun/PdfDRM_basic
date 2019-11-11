#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <string.h>
#include "sm2.h"

void Buf_Out (unsigned char *buf, int	buflen) //每32项为一行输出buf
{
	int i;
	printf ("\n");
	for (i = 0; i < buflen; i++) {
		if (i % 32 != 31)
			printf ("%02x", buf[i]);
		else
			printf ("%02x\n", buf[i]);
	}
	printf ("\n");
	return;
}
#define SEED_CONST 0x1BD8C95A
struct QXCS
{
	char *p;//椭圆曲线的参数
	char *a;
	char *b;
	char *n;  //G的阶
	char *x;   //g=(x,y)
	char *y;
};

struct QXCS pdf = { "FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF", "FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFC",
	"28E9FA9E9D9F5E344D5A9E4BCF6509A7F39789F515AB8F92DDBCBD414D940E93", "FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFF7203DF6B21C6052B53BBF40939D54123",
	"32C4AE2C1F1981195F9904466A39C9948FE30BBFF2660BE1715A4589334C74C7", "BC3736A2F4F6779C59BDCEE36B692153D0A9877CC62A474002DF32E52139F0A0",
};




//接收方B的私钥和公钥产生
void sm2_keygen (unsigned char *wx, int *wxlen, unsigned char *wy, int *wylen, unsigned char *privkey, int *privkeylen)
{
	struct QXCS *cfig = &pdf;
	epoint *g, *pB;
	big a, b, p, n, x, y, key1;
	miracl *mip = mirsys (20, 0);   //初始化大数系统
	mip->IOBASE = 16;   //输入为16进制数改为大数

	p = mirvar (0);
	a = mirvar (0);
	b = mirvar (0);
	n = mirvar (0);
	x = mirvar (0);
	y = mirvar (0);
	key1 = mirvar (0);

	cinstr (p, cfig->p);      //将大数字符串转换成大数,这里是16进制的字符串转换大数
	cinstr (a, cfig->a);
	cinstr (b, cfig->b);
	cinstr (n, cfig->n);
	cinstr (x, cfig->x);
	cinstr (y, cfig->y);

	ecurve_init (a, b, p, MR_PROJECTIVE);   //初始化椭圆曲线
	g = epoint_init ();
	pB = epoint_init ();
	epoint_set (x, y, 0, g);    //g=(x,y)为基点G
 //产生私钥
	irand (time (NULL) + SEED_CONST);   //初始化种子
	bigrand (n, key1);    //生成随机数key1
	ecurve_mult (key1, g, pB);   //pB=key1*g
	epoint_get (pB, x, y);    //取pB上的点（x，y）x和y即为公钥

	*wxlen = big_to_bytes (0, x, (char *)wx, FALSE);    //公钥写入wx，长度wxlen
	*wylen = big_to_bytes (0, y, (char *)wy, FALSE);
	*privkeylen = big_to_bytes (0, key1, (char *)privkey, FALSE);

	mirkill (key1);
	mirkill (p);
	mirkill (a);
	mirkill (b);
	mirkill (n);
	mirkill (x);
	mirkill (y);
	epoint_free (g);
	epoint_free (pB);
	mirexit ();
}


int kdf (unsigned char *zl, unsigned char *zr, int klen, unsigned char *kbuf)  //zl，zr为（x2，y2）
{

	unsigned char buf[70];
	unsigned char digest[32];
	unsigned int ct = 0x00000001;
	int i, m, n;
	unsigned char *p;

	memcpy (buf, zl, 32);                   //把x2，y2传入buf
	memcpy (buf + 32, zr, 32);

	m = klen / 32;
	n = klen % 32;
	p = kbuf;

	for (i = 0; i < m; i++)       //buf 64-70
	{
		buf[64] = (ct >> 24) & 0xFF;   //ct前8位
		buf[65] = (ct >> 16) & 0xFF;
		buf[66] = (ct >> 8) & 0xFF;
		buf[67] = ct & 0xFF;
		SM3Calc (buf, 68, p);                       //sm3后结果放在p中
		p += 32;
		ct++;
	}

	if (n != 0)
	{
		buf[64] = (ct >> 24) & 0xFF;
		buf[65] = (ct >> 16) & 0xFF;
		buf[66] = (ct >> 8) & 0xFF;
		buf[67] = ct & 0xFF;
		SM3Calc (buf, 68, digest);
	}

	memcpy (p, digest, n);

	for (i = 0; i < klen; i++)
	{
		if (kbuf[i] != 0)      //kbuf中有i+1个0
			break;
	}

	if (i < klen)
		return 1;   //kbuf（t）中的bit全是0， kdf判断通过，执行下一步C2=M异或t
	else
		return 0;

}

int A_encrypt (char *msg, int msglen, char *wx, int wxlen, char *wy, int wylen, char *outmsg)//wx，wy公钥的x，y的坐标
{
	struct QXCS *cfig = &pdf;
	big x2, y2, x1, y1, k;
	big a, b, p, n, x, y;
	epoint *g, *w, *pb, *c1, *kpb;
	int ret = -1;
	int i;
	unsigned char zl[32], zr[32];
	unsigned char *tmp;
	miracl *mip;
	tmp = malloc (msglen + 64);
	if (tmp == NULL)
		return -1;
	mip = mirsys (20, 0);
	mip->IOBASE = 16;          //读入16进制数

	p = mirvar (0);
	a = mirvar (0);
	b = mirvar (0);
	n = mirvar (0);
	x = mirvar (0);
	y = mirvar (0);
	k = mirvar (0);
	x2 = mirvar (0);
	y2 = mirvar (0);
	x1 = mirvar (0);
	y1 = mirvar (0);

	cinstr (p, cfig->p);                    //大数字符串变为大数
	cinstr (a, cfig->a);
	cinstr (b, cfig->b);
	cinstr (n, cfig->n);
	cinstr (x, cfig->x);
	cinstr (y, cfig->y);                                   //g=(x,y)

	ecurve_init (a, b, p, MR_PROJECTIVE);     //椭圆曲线方程初始化  y2 =x3 + Ax + B mod p
	g = epoint_init ();                                   //点坐标初始化
	pb = epoint_init ();
	kpb = epoint_init ();
	c1 = epoint_init ();
	w = epoint_init ();
	epoint_set (x, y, 0, g);                             //点坐标设置  g=(x,y)，现在无值
	bytes_to_big (wxlen, (char *)wx, x);       //把公钥wx和wy赋值给x，y
	bytes_to_big (wylen, (char *)wy, y);
	epoint_set (x, y, 0, pb);                          //=(x1,y1)


	  //选择小于n的随机数k
	irand (time (NULL) + SEED_CONST);
sm2_encrypt_again:
	do
	{
		bigrand (n, k);
	} while (k->len == 0);

	ecurve_mult (k, g, c1);                 //  点乘c1=k*g(第三个=第一个*第二个)
	epoint_get (c1, x1, y1);            //从c1里面得到x1，y1
	big_to_bytes (32, x1, (char *)outmsg, TRUE);
	big_to_bytes (32, y1, (char *)outmsg + 32, TRUE);


	if (point_at_infinity (pb))          //如果s是无穷点，返回1，报错退出
		goto exit_sm2_encrypt;

	ecurve_mult (k, pb, kpb);    //kpb=K*pb
	epoint_get (kpb, x2, y2);   //从kpb得到x2，y2


	big_to_bytes (32, x2, (char *)zl, TRUE);   //把大数x2，y2变为字节放入zl，zr
	big_to_bytes (32, y2, (char *)zr, TRUE);

	//t=KDF(x2||y2,klen)
	if (kdf (zl, zr, msglen, outmsg + 64) == 0)  //如果kdf返回的值为0，从头开始重新计算
		goto sm2_encrypt_again;

	for (i = 0; i < msglen; i++)
	{
		outmsg[64 + i] ^= msg[i];
	}

	//tmp=x2 || M| |y2 相连
	memcpy (tmp, zl, 32);
	memcpy (tmp + 32, msg, msglen);
	memcpy (tmp + 32 + msglen, zr, 32);

	//C3=outmsg=hash(SM3)(tmp)
	SM3Calc (tmp, 64 + msglen, &outmsg[64 + msglen]);
	ret = msglen + 64 + 32;

exit_sm2_encrypt:  //退出释放内存
	mirkill (x2);
	mirkill (y2);
	mirkill (x1);
	mirkill (y1);
	mirkill (k);
	mirkill (a);
	mirkill (b);
	mirkill (p);
	mirkill (n);
	mirkill (x);
	mirkill (y);
	epoint_free (g);   //释放点内存
	epoint_free (w);
	epoint_free (pb);
	epoint_free (kpb);
	mirexit ();
	free (tmp);
	return ret;
}

//B收到密文后开始解密运算
int decrypt (char *msg, int msglen, char *privkey, int privkeylen, char *outmsg)
{
	struct QXCS *cfig = &pdf;
	big x2, y2, c, k;
	big a, b, p, n, x, y, key1, dB;
	epoint *g, *C1, *dBC1;
	unsigned char c3[32];
	unsigned char zl[32], zr[32];
	int i, ret = -1;
	unsigned char *tmp;
	miracl *mip;
	if (msglen < 96)
		return 0;
	msglen -= 96;
	tmp = malloc (msglen + 64);
	if (tmp == NULL)
		return 0;
	mip = mirsys (20, 0);
	mip->IOBASE = 16;
	x2 = mirvar (0);
	y2 = mirvar (0);
	c = mirvar (0);
	k = mirvar (0);
	p = mirvar (0);
	a = mirvar (0);
	b = mirvar (0);
	n = mirvar (0);
	x = mirvar (0);
	y = mirvar (0);
	key1 = mirvar (0);
	dB = mirvar (0);
	bytes_to_big (privkeylen, (char *)privkey, dB);
	cinstr (p, cfig->p);
	cinstr (a, cfig->a);
	cinstr (b, cfig->b);
	cinstr (n, cfig->n);
	cinstr (x, cfig->x);
	cinstr (y, cfig->y);


	ecurve_init (a, b, p, MR_PROJECTIVE);  //初始化椭圆曲线 y2=x3+Ax+B （mod p）
	g = epoint_init ();
	dBC1 = epoint_init ();
	C1 = epoint_init ();
	bytes_to_big (32, (char *)msg, x);    //从msg中分别取出32位放入x和y
	bytes_to_big (32, (char *)msg + 32, y);

	if (!epoint_set (x, y, 0, C1))     //初始化点C1=（x，y）点C1=（x，y）是否在椭圆曲线 上
		goto exit_sm2_decrypt;
	if (point_at_infinity (C1))     //如果s（test）是无穷远点，报错并退出
		goto exit_sm2_decrypt;

	ecurve_mult (dB, C1, dBC1);   //dBC1=dB*c1
	epoint_get (dBC1, x2, y2);    //从dBC1中读取x2，y2

	big_to_bytes (32, x2, (char *)zl, TRUE);
	big_to_bytes (32, y2, (char *)zr, TRUE);

	if (kdf (zl, zr, msglen, outmsg) == 0)  //判断：t=kdf不是全0，才继续
		goto exit_sm2_decrypt;
	for (i = 0; i < msglen; i++)     //M'(outmsg)=C2 ^ t(outmsg)
	{
		outmsg[i] ^= msg[i + 64];//密文从65位开始为c2
	}
	memcpy (tmp, zl, 32);
	memcpy (tmp + 32, outmsg, msglen);
	memcpy (tmp + 32 + msglen, zr, 32);
	SM3Calc (tmp, 64 + msglen, c3);
	if (memcmp (c3, msg + 64 + msglen, 32) != 0)//判断u=c3则继续
		goto exit_sm2_decrypt;
	ret = msglen;

exit_sm2_decrypt:
	mirkill (x2);
	mirkill (y2);
	mirkill (c);
	mirkill (k);
	mirkill (p);
	mirkill (a);
	mirkill (b);
	mirkill (n);
	mirkill (x);
	mirkill (y);
	mirkill (key1);
	mirkill (dB);
	epoint_free (g);
	epoint_free (dBC1);
	mirexit ();
	free (tmp);

	return ret;
}




int main_test_sm2 ()
{
	unsigned char dB[32];   //存放私钥
	unsigned char xB[32];   //存放公钥pb（x，y）
	unsigned char yB[32];
	unsigned char tx[256] = "0";
	unsigned char etx[256];
	unsigned char mtx[256] = "0";
	int j;
	struct QXCS *cfig = &pdf;
	printf ("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<SM2椭圆曲线公钥密码>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	printf ("由pdf给出的参数：\n\tp:%s\n\ta:%s\n\tb:%s\n\tn:%s\n\tG(x):%s\n\tG(y):%s\n", cfig->p, cfig->a, cfig->b, cfig->n, cfig->x, cfig->y);
	FILE *fp;
	fopen_s (&fp, "7.txt", "r+");
	fgets (tx, 200, fp);
	fclose (fp);
	printf ("\n文件中明文为：\n\t%s\n", tx);
	int msglen = strlen (tx);
	int wxlen, wylen, privkeylen;
	sm2_keygen (xB, &wxlen, yB, &wylen, dB, &privkeylen);  //公钥（xB，yB）dB私钥 空值进入，产生公私钥
	printf ("\n\t公钥x坐标：");
	for (j = 0; j < wxlen; j++)
		printf ("%02x", xB[j]);
	printf ("\n\t公钥y坐标:");
	for (j = 0; j < wylen; j++)
		printf ("%02x", yB[j]);
	printf ("\n\t私钥:");
	for (j = 0; j < privkeylen; j++)
		printf ("%02x", dB[j]);
	printf ("\n");
	A_encrypt (tx, msglen, xB, 32, yB, 32, etx); //传入明文和公钥xb，yb
	printf ("加密结果: ");
	Buf_Out (etx, 64 + msglen + 32);
	decrypt (etx, 64 + msglen + 32, dB, 32, mtx);  //传入密文与私钥pb
	if (decrypt (etx, 64 + msglen + 32, dB, 32, mtx) < 0)
		printf ("sm2_decrypt error!\n");
	else {
		printf ("\n解密结果: ");
		Buf_Out (mtx, msglen);
		printf ("解密出的明文为：\n\t%s\n", mtx);
	}
	return 0;
}