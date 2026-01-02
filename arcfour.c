#include "arcfour.h"

export Arcfour *rc4init(int8 *key,int16 size){
    int16 x;
    int8 temp1;
    int8 temp2;
    Arcfour *p;

    int32 n;

    if(!(p=malloc(sizeof(struct s_arcfour))))
        {assert_perror(errno);}
    
        for (x=0;x<256;x++)
           p->s[x]=0;
        p->i=p->j=p->k=0;
        temp1=temp2=0;

    /*for i from 0 to 255
        S[i] := i
    endfor*/
        for(p->i=0;p->i <256;p->i++)
            p->s[p->i]=p->i;

/*
    j := 0 //we've already done this
    for i from 0 to 255

        j := (j + S[i] + key[i mod keylength]) mod 256
        swap values of S[i] and S[j]
    endfor*/
     for(p->i=0;p->i <256;p->i++){
        temp1=p->i % size;
        temp2=p->j+p->s[p->i]+ key[temp1];
        p->j=temp2 % 256;

        temp1=p->s[p->i];
        temp2=p->s[p->j];
        p->s[p->i]=temp2;
        p->s[p->j]=temp1;
        
     }

     p->i=p->j=0;
     rc4whitewash(n,p);
     return p;

}


int8 rc4byte(Arcfour *p){
    int16 tmp1,tmp2;

    /*while GeneratingOutput: we dont need this loop for now.. it will be handled in the encryption


    i := (i + 1) mod 256*/
    p->i=(p->i+1)%256;

    
    /*j := (j + S[i]) mod 256*/
    p->j=(p->j+p->s[p->i]) %256;


    //swap values of S[i] and S[j]
    tmp1=p->s[p->i];
    tmp2=p->s[p->j];

    p->s[p->i]=tmp2;
    p->s[p->j]=tmp1;



    //t := (S[i] + S[j]) mod 256
    tmp1=(p->s[p->i] +p->s[p->j]) % 256;


    //K := S[t]
    p->k=p->s[tmp1];
    
    //output K
    return p->k;

}

export int8 *rc4encrypt(Arcfour *p,int8 *cleartext,int16 size){
    int8 *ciphertext;
     int16 x;
    ciphertext=(int8 *)malloc(size+1);
    if(!ciphertext){
        assert_perror(errno);
    }

    for(x=0;x<size;x++){
        ciphertext[x]=cleartext[x]^ rc4byte(p);
        }
    
    return ciphertext;

}
