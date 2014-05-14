
int main(int argc, char *argv[])
{
				nivel = qm.ValorAna[tabindex]*BdConf.anaconf.fcm[tabindex]+BdConf.anaconf.fca[tabindex];
				*((unsigned char *)&table[i]) = (unsigned char)*((unsigned char *)&nivel   );
				*(1+(unsigned char *)&table[i++]) = (unsigned char)*((unsigned char *)&nivel +1);
				*((unsigned char *)&table[i]) = (unsigned char)*((unsigned char *)&nivel +2);
				*(1+(unsigned char *)&table[i++]) = (unsigned char)*((unsigned char *)&nivel +3);
				//printf("\nEnvio Axis QM.Ing %d valor:%f 1(%d) 2(%d)\n",tabindex,nivel,table[i-2],table[i-1]);	
exit(0);
}
