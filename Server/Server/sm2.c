#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <string.h>
#include "sm2.h"

void Buf_Out (unsigned char *buf, int	buflen) //ÿ32��Ϊһ�����buf
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
	char *p;//��Բ���ߵĲ���
	char *a;
	char *b;
	char *n;  //G�Ľ�
	char *x;   //g=(x,y)
	char *y;
};

struct QXCS pdf = { "FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF", "FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFC",
	"28E9FA9E9D9F5E344D5A9E4BCF6509A7F39789F515AB8F92DDBCBD414D940E93", "FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFF7203DF6B21C6052B53BBF40939D54123",
	"32C4AE2C1F1981195F9904466A39C9948FE30BBFF2660BE1715A4589334C74C7", "BC3736A2F4F6779C59BDCEE36B692153D0A9877CC62A474002DF32E52139F0A0",
};




//���շ�B��˽Կ�͹�Կ����
void sm2_keygen (unsigned char *wx, int *wxlen, unsigned char *wy, int *wylen, unsigned char *privkey, int *privkeylen)
{
	struct QXCS *cfig = &pdf;
	epoint *g, *pB;
	big a, b, p, n, x, y, key1;
	miracl *mip = mirsys (20, 0);   //��ʼ������ϵͳ
	mip->IOBASE = 16;   //����Ϊ16��������Ϊ����

	p = mirvar (0);
	a = mirvar (0);
	b = mirvar (0);
	n = mirvar (0);
	x = mirvar (0);
	y = mirvar (0);
	key1 = mirvar (0);

	cinstr (p, cfig->p);      //�������ַ���ת���ɴ���,������16���Ƶ��ַ���ת������
	cinstr (a, cfig->a);
	cinstr (b, cfig->b);
	cinstr (n, cfig->n);
	cinstr (x, cfig->x);
	cinstr (y, cfig->y);

	ecurve_init (a, b, p, MR_PROJECTIVE);   //��ʼ����Բ����
	g = epoint_init ();
	pB = epoint_init ();
	epoint_set (x, y, 0, g);    //g=(x,y)Ϊ����G
 //����˽Կ
	irand (time (NULL) + SEED_CONST);   //��ʼ������
	bigrand (n, key1);    //���������key1
	ecurve_mult (key1, g, pB);   //pB=key1*g
	epoint_get (pB, x, y);    //ȡpB�ϵĵ㣨x��y��x��y��Ϊ��Կ

	*wxlen = big_to_bytes (0, x, (char *)wx, FALSE);    //��Կд��wx������wxlen
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


int kdf (unsigned char *zl, unsigned char *zr, int klen, unsigned char *kbuf)  //zl��zrΪ��x2��y2��
{

	unsigned char buf[70];
	unsigned char digest[32];
	unsigned int ct = 0x00000001;
	int i, m, n;
	unsigned char *p;

	memcpy (buf, zl, 32);                   //��x2��y2����buf
	memcpy (buf + 32, zr, 32);

	m = klen / 32;
	n = klen % 32;
	p = kbuf;

	for (i = 0; i < m; i++)       //buf 64-70
	{
		buf[64] = (ct >> 24) & 0xFF;   //ctǰ8λ
		buf[65] = (ct >> 16) & 0xFF;
		buf[66] = (ct >> 8) & 0xFF;
		buf[67] = ct & 0xFF;
		SM3Calc (buf, 68, p);                       //sm3��������p��
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
		if (kbuf[i] != 0)      //kbuf����i+1��0
			break;
	}

	if (i < klen)
		return 1;   //kbuf��t���е�bitȫ��0�� kdf�ж�ͨ����ִ����һ��C2=M���t
	else
		return 0;

}

int A_encrypt (char *msg, int msglen, char *wx, int wxlen, char *wy, int wylen, char *outmsg)//wx��wy��Կ��x��y������
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
	mip->IOBASE = 16;          //����16������

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

	cinstr (p, cfig->p);                    //�����ַ�����Ϊ����
	cinstr (a, cfig->a);
	cinstr (b, cfig->b);
	cinstr (n, cfig->n);
	cinstr (x, cfig->x);
	cinstr (y, cfig->y);                                   //g=(x,y)

	ecurve_init (a, b, p, MR_PROJECTIVE);     //��Բ���߷��̳�ʼ��  y2 =x3 + Ax + B mod p
	g = epoint_init ();                                   //�������ʼ��
	pb = epoint_init ();
	kpb = epoint_init ();
	c1 = epoint_init ();
	w = epoint_init ();
	epoint_set (x, y, 0, g);                             //����������  g=(x,y)��������ֵ
	bytes_to_big (wxlen, (char *)wx, x);       //�ѹ�Կwx��wy��ֵ��x��y
	bytes_to_big (wylen, (char *)wy, y);
	epoint_set (x, y, 0, pb);                          //=(x1,y1)


	  //ѡ��С��n�������k
	irand (time (NULL) + SEED_CONST);
sm2_encrypt_again:
	do
	{
		bigrand (n, k);
	} while (k->len == 0);

	ecurve_mult (k, g, c1);                 //  ���c1=k*g(������=��һ��*�ڶ���)
	epoint_get (c1, x1, y1);            //��c1����õ�x1��y1
	big_to_bytes (32, x1, (char *)outmsg, TRUE);
	big_to_bytes (32, y1, (char *)outmsg + 32, TRUE);


	if (point_at_infinity (pb))          //���s������㣬����1�������˳�
		goto exit_sm2_encrypt;

	ecurve_mult (k, pb, kpb);    //kpb=K*pb
	epoint_get (kpb, x2, y2);   //��kpb�õ�x2��y2


	big_to_bytes (32, x2, (char *)zl, TRUE);   //�Ѵ���x2��y2��Ϊ�ֽڷ���zl��zr
	big_to_bytes (32, y2, (char *)zr, TRUE);

	//t=KDF(x2||y2,klen)
	if (kdf (zl, zr, msglen, outmsg + 64) == 0)  //���kdf���ص�ֵΪ0����ͷ��ʼ���¼���
		goto sm2_encrypt_again;

	for (i = 0; i < msglen; i++)
	{
		outmsg[64 + i] ^= msg[i];
	}

	//tmp=x2 || M| |y2 ����
	memcpy (tmp, zl, 32);
	memcpy (tmp + 32, msg, msglen);
	memcpy (tmp + 32 + msglen, zr, 32);

	//C3=outmsg=hash(SM3)(tmp)
	SM3Calc (tmp, 64 + msglen, &outmsg[64 + msglen]);
	ret = msglen + 64 + 32;

exit_sm2_encrypt:  //�˳��ͷ��ڴ�
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
	epoint_free (g);   //�ͷŵ��ڴ�
	epoint_free (w);
	epoint_free (pb);
	epoint_free (kpb);
	mirexit ();
	free (tmp);
	return ret;
}

//B�յ����ĺ�ʼ��������
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


	ecurve_init (a, b, p, MR_PROJECTIVE);  //��ʼ����Բ���� y2=x3+Ax+B ��mod p��
	g = epoint_init ();
	dBC1 = epoint_init ();
	C1 = epoint_init ();
	bytes_to_big (32, (char *)msg, x);    //��msg�зֱ�ȡ��32λ����x��y
	bytes_to_big (32, (char *)msg + 32, y);

	if (!epoint_set (x, y, 0, C1))     //��ʼ����C1=��x��y����C1=��x��y���Ƿ�����Բ���� ��
		goto exit_sm2_decrypt;
	if (point_at_infinity (C1))     //���s��test��������Զ�㣬�����˳�
		goto exit_sm2_decrypt;

	ecurve_mult (dB, C1, dBC1);   //dBC1=dB*c1
	epoint_get (dBC1, x2, y2);    //��dBC1�ж�ȡx2��y2

	big_to_bytes (32, x2, (char *)zl, TRUE);
	big_to_bytes (32, y2, (char *)zr, TRUE);

	if (kdf (zl, zr, msglen, outmsg) == 0)  //�жϣ�t=kdf����ȫ0���ż���
		goto exit_sm2_decrypt;
	for (i = 0; i < msglen; i++)     //M'(outmsg)=C2 ^ t(outmsg)
	{
		outmsg[i] ^= msg[i + 64];//���Ĵ�65λ��ʼΪc2
	}
	memcpy (tmp, zl, 32);
	memcpy (tmp + 32, outmsg, msglen);
	memcpy (tmp + 32 + msglen, zr, 32);
	SM3Calc (tmp, 64 + msglen, c3);
	if (memcmp (c3, msg + 64 + msglen, 32) != 0)//�ж�u=c3�����
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
	unsigned char dB[32];   //���˽Կ
	unsigned char xB[32];   //��Ź�Կpb��x��y��
	unsigned char yB[32];
	unsigned char tx[256] = "0";
	unsigned char etx[256];
	unsigned char mtx[256] = "0";
	int j;
	struct QXCS *cfig = &pdf;
	printf ("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<SM2��Բ���߹�Կ����>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	printf ("��pdf�����Ĳ�����\n\tp:%s\n\ta:%s\n\tb:%s\n\tn:%s\n\tG(x):%s\n\tG(y):%s\n", cfig->p, cfig->a, cfig->b, cfig->n, cfig->x, cfig->y);
	FILE *fp;
	fopen_s (&fp, "7.txt", "r+");
	fgets (tx, 200, fp);
	fclose (fp);
	printf ("\n�ļ�������Ϊ��\n\t%s\n", tx);
	int msglen = strlen (tx);
	int wxlen, wylen, privkeylen;
	sm2_keygen (xB, &wxlen, yB, &wylen, dB, &privkeylen);  //��Կ��xB��yB��dB˽Կ ��ֵ���룬������˽Կ
	printf ("\n\t��Կx���꣺");
	for (j = 0; j < wxlen; j++)
		printf ("%02x", xB[j]);
	printf ("\n\t��Կy����:");
	for (j = 0; j < wylen; j++)
		printf ("%02x", yB[j]);
	printf ("\n\t˽Կ:");
	for (j = 0; j < privkeylen; j++)
		printf ("%02x", dB[j]);
	printf ("\n");
	A_encrypt (tx, msglen, xB, 32, yB, 32, etx); //�������ĺ͹�Կxb��yb
	printf ("���ܽ��: ");
	Buf_Out (etx, 64 + msglen + 32);
	decrypt (etx, 64 + msglen + 32, dB, 32, mtx);  //����������˽Կpb
	if (decrypt (etx, 64 + msglen + 32, dB, 32, mtx) < 0)
		printf ("sm2_decrypt error!\n");
	else {
		printf ("\n���ܽ��: ");
		Buf_Out (mtx, msglen);
		printf ("���ܳ�������Ϊ��\n\t%s\n", mtx);
	}
	return 0;
}