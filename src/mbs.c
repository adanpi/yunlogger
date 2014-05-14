#include "modbus.h"
#include "modbus_tcp.h"
#include <sys/socket.h>
#include <time.h>
#include <errno.h>
#include "pantalla.h"

int Mbs_slave;					/* slave number */
pthread_t Mbs_thread_tcp_aux;	/* slave thread */
int Mbs_device;								/* slave device */
int Mbs_socket;								/* socket esclavo principal*/
int socketActual;							/* socket tcp nueva conexion*/
pthread_mutex_t mutex;						/* cerrojo para sincronizar accesos a ciertas partes*/

char aux[64];
unsigned long segjulact,time_ultimo_char;
struct tm *newtime;
char *auxch;
short primer_char=1;


/***********************************************************************************
   	   Mbs_read : read one data and call pointer function if !=NULL
 ************************************************************************************
input :
-------
unsigned char c : pointer of the char

no output
 ***********************************************************************************/
void Mbs_read(unsigned char *c)
{
	read(Mbs_device,c,1);
	if(Mb_ptr_rcv_data!=NULL)
		(*Mb_ptr_rcv_data)(*c);
}


/***********************************************************************************
   	   Mbs_write: write packet and call pointer function if !=NULL
 ************************************************************************************
input :
-------
trame 	: packet to send
longeur	: length of the packet

no output
 ***********************************************************************************/
void Mbs_write(unsigned char trame[256], int longeur)
{
	int i;
	for(i=0;i<longeur;i++)
	{
		write(Mbs_device,&trame[i],1);
		if(Mb_ptr_snd_data!=NULL)
			(*Mb_ptr_snd_data)(trame[i]);
	}
}

void ponSocket(int socket){
	socketActual=socket;
}

int dameSocket(){
	return socketActual;
}

void MensajePosicionDireccionNoExiste(){
	segjulact=time(NULL);                                   // Hora Actual
	newtime=localtime(&segjulact);
	auxch=asctime(newtime);
	sprintf(aux,"%s\t Peticion Direccion No Existente en Axis (Max Ana: %d) (Max Dig: %d)... Finalizar\n",auxch,POSDIGMODBUS,(BBDDMODBUSLON-POSDIGMODBUS)*8);
	AxisLog(aux);
	if (Mb_verbose) fprintf(stderr,"%s",aux);
}

void ponHilo(pthread_t Mbs_thread_tcp){
	Mbs_thread_tcp_aux=Mbs_thread_tcp;
}
pthread_t dameHilo(){
	return Mbs_thread_tcp_aux;
}

/**********************************************************************************
   	   Mbs : Main slave function
 ***********************************************************************************
input not used
no output
 **********************************************************************************/


void Mbs_tcp_conexion_nueva(void *ptr){

	// Inicio Variables a usar en funcion que se ejecutará como hilo

	// socket activo en este hilo
	int Mbs_socket_nuevo=-1;
	// identificador del hilo
	pthread_t Mbs_thread_tcp_conexion;
	int i,j,bytes_mb,FunModBus=0,DirModBus=0,NumRegModBus=0,EscModBus=0,DatModBusRec[MAX_DATA_LENGTH];
	unsigned char byte1, byte2,auxuch1,auxuch2;
	unsigned char ModBusEnv[MAX_RESPONSE_LENGTH];
	//unsigned char *ModBusEnv;
	//memset (&ModBusEnv,0,MAX_RESPONSE_LENGTH);
	//ModBusEnv=calloc(MAX_RESPONSE_LENGTH,MAX_RESPONSE_LENGTH*sizeof(unsigned char));
	//free(ModBusEnv);
	short finalizar=0;

	// Fin Variables Hilo

	//Mbs_socket_nuevo=dameSocket();
	Mbs_socket_nuevo=*(int *) ptr;
	//Mbs_thread_tcp_conexion=*(pthread_t *) ptr;
	Mbs_thread_tcp_conexion=pthread_self();

	do{

		pthread_mutex_lock(&mutex);
		finalizar=0;
		bytes_mb=get_slave_query( &byte1, &byte2, &EscModBus, &FunModBus, &DirModBus, &NumRegModBus, &DatModBusRec, Mbs_socket_nuevo);

		if (bytes_mb > 0){
			if (Mb_verbose){
				fprintf(stderr,"ModBus TCP recibidos: %d bytes \n",bytes_mb);
				fprintf( stderr, "Mbs: Esclavo: %d, Funcion: %d, Dir: %d, Num: %d \n", EscModBus, FunModBus, DirModBus, NumRegModBus);
			}


			if ( EscModBus == Mbs_slave)
			{
#ifdef DEBUG
				fprintf(stderr,"Petición del Master TCP\n");
				fprintf(stderr,"function %d \n",FunModBus);
#endif
				switch (FunModBus)
				{
				/*********************************************************************/
				// leer estados bit
				case 1:
				case 2:
					if( (DirModBus+NumRegModBus) > (BBDDMODBUSLON-POSDIGMODBUS)*8 ){
						MensajePosicionDireccionNoExiste();
						break;
					}
					// Se construye mensaje para enviar:
					ModBusEnv[0]=byte1;
					ModBusEnv[1]=byte2;
					ModBusEnv[5]=(unsigned char)(4 + ((NumRegModBus-1)/8) );
					ModBusEnv[6]=(unsigned char)(EscModBus);
					ModBusEnv[7]=(unsigned char)(FunModBus);
					ModBusEnv[8]=(unsigned char)(1+((NumRegModBus-1)/8));
					for (i=0;i<1+((NumRegModBus-1)/8);i++){
						//tomamos el primer byte y desplazamos izquierda desde la direccion solicitada
						auxuch1=Mbs_data[POSDIGMODBUS+(DirModBus/8)+i]<<(DirModBus%8);
						//tomamos siguiente byte desplazados ala derecha desde la direccion solicitada
						auxuch2=(Mbs_data[POSDIGMODBUS+(DirModBus/8)+1+i]>>(8-(DirModBus%8)));
						//OR
						ModBusEnv[9+i]=auxuch1 | auxuch2;
						//revertir orden
						ModBusEnv[9+i] = (ModBusEnv[9+i] * 0x0202020202ULL & 0x010884422010ULL) % 1023;

#ifdef DEBUG
						for (j=7;j>=0;j--)
							fprintf(stderr,"%d",auxuch1>>j & 1);
						fprintf(stderr,"\n");
						for (j=7;j>=0;j--)
							fprintf(stderr,"%d",auxuch2>>j & 1);
						fprintf(stderr,"\n");
#endif
					}

					//envio del mensaje:
					bytes_mb=send_query( Mbs_socket_nuevo, &ModBusEnv, ((NumRegModBus-1)/8)+10 );
					if(bytes_mb == -1)
						fprintf(stderr,"Error en Envio, funcion: %d \n",FunModBus);
					FunModBus=0;DirModBus=0;NumRegModBus=0;EscModBus=0;
					break;
					/*********************************************************************/
					// leer uno o varios bytes
				case 3:
				case 4:
					if( (DirModBus+NumRegModBus) > POSDIGMODBUS){
						MensajePosicionDireccionNoExiste();
						break;
					}
#ifdef DEBUG
					fprintf(stderr,"adress %d \n",DirModBus);
					fprintf(stderr,"lenght %d \n",NumRegModBus);
					for(i=0;i<bytes_mb;i++){
						fprintf(stderr,"Datos[%d] = %d \n",i,DatModBusRec[i]);
					}
#endif
					// Se construye mensaje para enviar:
					ModBusEnv[0]=byte1;
					ModBusEnv[1]=byte2;
					ModBusEnv[5]=(unsigned char)(3+NumRegModBus*2);
					ModBusEnv[6]=(unsigned char)(EscModBus);
					ModBusEnv[7]=(unsigned char)(FunModBus);
					for (i=0;i<NumRegModBus;i++)
					{
#ifdef DEBUG
						fprintf(stderr,"Mbs_data[%d] = %0x\n",DirModBus+i,Mbs_data[DirModBus+i]);
#endif
						ModBusEnv[9+i*2]=Mbs_data[DirModBus+i]>>8;
						ModBusEnv[10+i*2]=Mbs_data[DirModBus+i]&0xff;
					}
					ModBusEnv[8]=(unsigned char)(NumRegModBus*2);
					//envio del mensaje:
					bytes_mb=send_query( Mbs_socket_nuevo, &ModBusEnv, (NumRegModBus*2)+9 );

					if(bytes_mb == -1)
						fprintf(stderr,"Error en Envio, funcion: %d \n",FunModBus);
					FunModBus=0;DirModBus=0;NumRegModBus=0;EscModBus=0;
					break;
					/*********************************************************************/
					// escribir un registro 1 bit
				case 5:
					if( (DirModBus+NumRegModBus) > (BBDDMODBUSLON-POSDIGMODBUS)*8 ){
						MensajePosicionDireccionNoExiste();
						break;
					}

					// Se escribe bit correspondiente:
					if((unsigned char)(DatModBusRec[0])==0xff){
						Mbs_data[POSDIGMODBUS+(DirModBus/8)]|= 1 << (7-(DirModBus%8));
					}else{
						Mbs_data[POSDIGMODBUS+(DirModBus/8)]&= ~(1 << (7-(DirModBus%8)) );
					}

#ifdef DEBUG
					auxuch1=Mbs_data[POSDIGMODBUS+(DirModBus/8)];
					fprintf(stderr,"auxch1 [%02X] Mbs_data[%d] [%02X]\n",auxuch1,POSDIGMODBUS+(DirModBus/8),Mbs_data[POSDIGMODBUS+(DirModBus/8)]);
					for (i=7;i>=0;i--)
						fprintf(stderr,"%d",auxuch1>>i & 1);
#endif
					// Se construye mensaje para enviar:
					ModBusEnv[0]=byte1;
					ModBusEnv[1]=byte2;
					ModBusEnv[5]=(unsigned char)((short)6);
					ModBusEnv[6]=(unsigned char)(EscModBus);
					ModBusEnv[7]=(unsigned char)(FunModBus);
					ModBusEnv[8]=(unsigned char)(DirModBus>>8);
					ModBusEnv[9]=(unsigned char)(DirModBus&0xff);
					ModBusEnv[10]=(unsigned char)(DatModBusRec[0]);
					ModBusEnv[11]=0x00;

#ifdef DEBUG
					fprintf(stderr,"\nadress %d \n",DirModBus);
					for(i=0;i<bytes_mb;i++){
						fprintf(stderr,"Datos[%d] = %d \n",i,DatModBusRec[i]);
					}
#endif

					//envio del mensaje:
					bytes_mb=send_query( Mbs_socket_nuevo, &ModBusEnv, 12 );
					if(bytes_mb == -1)
						fprintf(stderr,"Error en Envio, funcion: %d \n",FunModBus);
					FunModBus=0;DirModBus=0;NumRegModBus=0;EscModBus=0;
					break;
					/*********************************************************************/
					// escribir un registro byte
				case 6:
					if( (DirModBus+NumRegModBus) > POSDIGMODBUS){
						MensajePosicionDireccionNoExiste();
						break;
					}
#ifdef DEBUG
					fprintf(stderr,"adress %d \n",DirModBus);
					for(i=0;i<bytes_mb;i++){
						fprintf(stderr,"Datos[%d] = %d \n",i,DatModBusRec[i]);
					}
#endif
					Mbs_data[DirModBus]=DatModBusRec[0];


					// Se construye mensaje para enviar:
					ModBusEnv[0]=byte1;
					ModBusEnv[1]=byte2;
					ModBusEnv[5]=(unsigned char)((short)6);
					ModBusEnv[6]=(unsigned char)(EscModBus);
					ModBusEnv[7]=(unsigned char)(FunModBus);
					ModBusEnv[8]=(unsigned char)(DirModBus>>8);
					ModBusEnv[9]=(unsigned char)(DirModBus&0xff);
#ifdef DEBUG
					fprintf(stderr,"Mbs_data[%d] = %d\n",DirModBus,Mbs_data[DirModBus]);
#endif
					ModBusEnv[10]=Mbs_data[DirModBus]>>8;
					ModBusEnv[11]=Mbs_data[DirModBus]&0xff;

					//envio del mensaje:
					bytes_mb=send_query( Mbs_socket_nuevo, &ModBusEnv, 12 );

					if(bytes_mb == -1)
						fprintf(stderr,"Error en Envio, funcion: %d \n",FunModBus);
					FunModBus=0;DirModBus=0;NumRegModBus=0;EscModBus=0;
					break;
					/*********************************************************************/
					// escribir varios registros 1 bit
				case 15:
					if( (DirModBus+NumRegModBus) > (BBDDMODBUSLON-POSDIGMODBUS)*8 ){
						MensajePosicionDireccionNoExiste();
						break;
					}

					// Se escriben bit correspondientes:
					auxuch1=Mbs_data[POSDIGMODBUS+(DirModBus/8)];

					auxuch1=auxuch1>>(8-(DirModBus%8));
					auxuch1=auxuch1<<(8-(DirModBus%8));

					//revertir orden byte recibido
					DatModBusRec[0] = (DatModBusRec[0] * 0x0202020202ULL & 0x010884422010ULL) % 1023;

					auxuch2=DatModBusRec[0]>>(DirModBus%8);

					Mbs_data[POSDIGMODBUS+(DirModBus/8)]=auxuch1 | auxuch2;
					auxuch1=DatModBusRec[0]<<(8-(DirModBus%8));

					for (i=1;i<1+((NumRegModBus-1)/8);i++){
						//revertir orden byte recibido
						DatModBusRec[i] = (DatModBusRec[i] * 0x0202020202ULL & 0x010884422010ULL) % 1023;
						auxuch2=DatModBusRec[i]>>(DirModBus%8);
						Mbs_data[POSDIGMODBUS+(DirModBus/8)+i]=auxuch1 | auxuch2;
						auxuch1=DatModBusRec[i]<<(8-(DirModBus%8));
					}
					Mbs_data[POSDIGMODBUS+(DirModBus/8)+i]=auxuch1 | Mbs_data[POSDIGMODBUS+(DirModBus/8)+i];
					// Se construye mensaje para enviar:
					ModBusEnv[0]=byte1;
					ModBusEnv[1]=byte2;
					ModBusEnv[5]=(unsigned char)((short)6);
					ModBusEnv[6]=(unsigned char)(EscModBus);
					ModBusEnv[7]=(unsigned char)(FunModBus);
					ModBusEnv[8]=(unsigned char)(DirModBus>>8);
					ModBusEnv[9]=(unsigned char)(DirModBus&0xff);
					ModBusEnv[10]=(unsigned char)(NumRegModBus>>8);
					ModBusEnv[11]=(unsigned char)(NumRegModBus&0xff);


					//envio del mensaje:
					bytes_mb=send_query( Mbs_socket_nuevo, &ModBusEnv, 12 );
					if(bytes_mb == -1)
						fprintf(stderr,"Error en Envio, funcion: %d \n",FunModBus);
					FunModBus=0;DirModBus=0;NumRegModBus=0;EscModBus=0;
					break;
					// escribir varios registros
				case 16:
					if( (DirModBus+NumRegModBus) > POSDIGMODBUS){
						MensajePosicionDireccionNoExiste();
						break;
					}
					// se escriben los registros:
					for (i=0;i<NumRegModBus;i++)
						Mbs_data[DirModBus+i]=DatModBusRec[i];
#ifdef DEBUG
					fprintf(stderr,"Desde direccion %d se escriben %d registros\n",DirModBus,NumRegModBus);
					for(i=0;i<NumRegModBus;i++){
						fprintf(stderr,"Mbs_data[%d] = %d \n",DirModBus+i,Mbs_data[DirModBus+i]);
					}
#endif
					// Se construye mensaje para enviar:
					ModBusEnv[0]=byte1;
					ModBusEnv[1]=byte2;
					ModBusEnv[5]=(unsigned char)((short)6);
					ModBusEnv[6]=(unsigned char)(EscModBus);
					ModBusEnv[7]=(unsigned char)(FunModBus);
					ModBusEnv[8]=(unsigned char)(DirModBus>>8);
					ModBusEnv[9]=(unsigned char)(DirModBus&0xff);
					ModBusEnv[10]=(unsigned char)(NumRegModBus>>8);
					ModBusEnv[11]=(unsigned char)(NumRegModBus&0xff);
					//envio del mensaje:
					bytes_mb=send_query( Mbs_socket_nuevo, &ModBusEnv, 12 );

					if(bytes_mb == -1)
						fprintf(stderr,"Error en Envio, funcion: %d \n",FunModBus);
					FunModBus=0;DirModBus=0;NumRegModBus=0;EscModBus=0;
					break;

				}
			}else{
				if (Mb_verbose) fprintf(stderr,"Petición del Master para otro esclavo... Finalizar\n");
				finalizar=1;
				sprintf(aux,"%s\tPetición del Master para otro esclavo..",auxch);
				AxisLog(aux);
			}
		}
		else{
			if (Mb_verbose) fprintf(stderr,"No hay lectura de bytes en el socket...\n Finalizar Hilo %d Cerrar Socket %d\n",(int)Mbs_thread_tcp_conexion,Mbs_socket_nuevo);
			finalizar=1;
		}

		pthread_mutex_unlock(&mutex);
		//sched_yield();
	}while(finalizar == 0);
	close(Mbs_socket_nuevo);
	pthread_mutex_unlock(&mutex);
	pthread_cancel(Mbs_thread_tcp_conexion);
	pthread_detach(Mbs_thread_tcp_conexion);
	pthread_exit(NULL);

	if (Mb_verbose) fprintf(stderr,"\n\t\t Fin Hilo: %d \n",(int)Mbs_thread_tcp_conexion);
}

void Mbs_tcp(void *ptr){

	if (Mb_verbose) fprintf(stderr,"Escuchando Conexiones tcp \n");
	int Mbs_socket_nueva_com=-1;
	do{
		/* socket par cada nueva conexion*/
		Mbs_socket_nueva_com = accept( Mbs_socket, NULL, NULL );
		segjulact=time(NULL);                                   // Hora Actual
		newtime=localtime(&segjulact);
		auxch=asctime(newtime);
		if( Mbs_socket_nueva_com < 0 )
		{
			sprintf(aux,"%s\tProceso axismbus Error nueva conexion. errno %d",auxch,errno);
			AxisLog(aux);
			close( Mbs_socket );

		}else{
			//ponSocket(Mbs_socket_nueva_com);
			pthread_t Mbs_thread_tcp_con;
			//ponHilo(Mbs_thread_tcp_con);
			pthread_create(&Mbs_thread_tcp_con, NULL,(void*)&Mbs_tcp_conexion_nueva,(void*)&Mbs_socket_nueva_com);
			if (Mb_verbose) fprintf(stderr,"Nueva Conexion tcp! socket: %d hilo %d\n",Mbs_socket_nueva_com,(int)Mbs_thread_tcp_con);
		}
	}while(1);
}

void Mbs_hora(void *ptr){
	unsigned long segjul;
	struct tm *horaactual;
	if (Mb_verbose) fprintf(stderr,"Hilo hora modbus iniciado\n");

	do{
		segjul=time(NULL);                                   // Hora Actual
		horaactual=localtime(&segjul);


		if (Mb_verbose) fprintf(stderr,"\nHora ModBus: %02d/%02d/%02d %02d:%02d",horaactual->tm_mday,horaactual->tm_mon+1,horaactual->tm_year+1900,horaactual->tm_hour,horaactual->tm_min);

		if( segjul > (time_ultimo_char+TIMEOUT_MBS_RTU)){
			if (Mb_verbose) fprintf(stderr,"\nHora ModBus unlock MuTex");
			pthread_mutex_unlock(&mutex);
		}

		pthread_mutex_lock(&mutex);
		Mbs_data[POS_MB_MIN]=horaactual->tm_min;
		Mbs_data[POS_MB_HORA]=horaactual->tm_hour;
		Mbs_data[POS_MB_DIA]=horaactual->tm_mday;
		Mbs_data[POS_MB_MES]=horaactual->tm_mon+1;
		Mbs_data[POS_MB_ANIO]=horaactual->tm_year+1900;
		pthread_mutex_unlock(&mutex);


		TimeWait(REFRESCO_HORA);		// esperamos REFRESCO_HORA decimas segundos y volvemos a poner fecha
	}while(1);
}

void Mbs_rtu(void *ptr)
{
	unsigned char fonction,c,c1,c2,trame[256];
	int adresse,longueur,data,i,j;
	int data_to_write[255];
	primer_char=1;
	if (Mb_verbose) fprintf(stderr,"Escuchando Conexiones RTU \n");
	do {
		Mbs_read(&c);
		if (c==Mbs_slave)
		{
			if (Mb_verbose) fprintf(stderr,"Master call me rtu!\n");
			Mbs_read(&fonction);
			if (Mb_verbose) fprintf(stderr,"function 0x%x \n",fonction);

			if (Mb_verbose)
				fprintf(stderr,"\n\nBloquear mutex RTU \n");
			pthread_mutex_lock(&mutex);

			trame[0]=Mbs_slave;
			trame[1]=fonction;
			switch (fonction)
			{
			/*********************************************************************/
			case 0x01:
			case 0x02:
				/* leer un bit */
				/* get adress */
				Mbs_read(&c1);
				Mbs_read(&c2);
				adresse=(c1<<8)+c2;
				trame[2]=c1;
				trame[3]=c2;
				if (Mb_verbose) fprintf(stderr,"adress %d (%x %x) \n",adresse,c1,c2);

				/* get length */
				Mbs_read(&c1);
				Mbs_read(&c2);
				longueur=(c1<<8)+c2;
				if (Mb_verbose) fprintf(stderr,"lenght %d \n",longueur);
				trame[4]=c1;
				trame[5]=c2;
				if( (adresse+longueur) > (BBDDMODBUSLON-POSDIGMODBUS)*8 ){
					if (Mb_verbose) fprintf(stderr,"Peticion Posicion No Existente en Axis (Max: %d)... Finalizar\n",POSDIGMODBUS);
					if (Mb_verbose)
						for(i=0;i<=7;i++)
							fprintf(stderr,"sended packet[%d] = %0x\n",i,trame[i]);
					break;
				}
				/* get crc16 */
				Mbs_read(&c1);
				Mbs_read(&c2);
				trame[6]=c1;
				trame[7]=c2;
				if (Mb_verbose) fprintf(stderr,"crc  %x%x \n",c1,c2);
				/* check crc16 */
				if (Mb_test_crc(trame,6))
				{
					if (Mb_verbose) fprintf(stderr,"crc error\n");
					if(Mb_ptr_end_slve!=NULL)
						(*Mb_ptr_end_slve)(-1,0,0);
					break;
				}
				if (Mb_verbose)
					for(i=0;i<=7;i++)
						fprintf(stderr,"sended packet[%d] = %0x\n",i,trame[i]);


				if( (adresse+longueur) > (BBDDMODBUSLON-POSDIGMODBUS)*8 ){
					if (Mb_verbose) fprintf(stderr,"Peticion Posicion No Existente en Axis (Max: %d)... Finalizar\n",(BBDDMODBUSLON-POSDIGMODBUS)*8);
					if (Mb_verbose)
						for(i=0;i<=7;i++)
							fprintf(stderr,"sended packet[%d] = %0x\n",i,trame[i]);
					break;
				}
				// Se construye mensaje para enviar:
				trame[2]=(unsigned char)(1+((longueur-1)/8));
				for (i=0;i<1+((longueur-1)/8);i++){
					//tomamos el primer byte y desplazamos izquierda desde la direccion solicitada
					c1=Mbs_data[POSDIGMODBUS+(adresse/8)+i]<<(adresse%8);
					//tomamos siguiente byte desplazados ala derecha desde la direccion solicitada
					c2=(Mbs_data[POSDIGMODBUS+(adresse/8)+1+i]>>(8-(adresse%8)));
					//OR
					trame[3+i]=c1 | c2;
					//revertir orden
					trame[3+i] = (trame[3+i] * 0x0202020202ULL & 0x010884422010ULL) % 1023;

#ifdef DEBUG
					for (j=7;j>=0;j--)
						fprintf(stderr,"%d",c1>>j & 1);
					fprintf(stderr,"\n");
					for (j=7;j>=0;j--)
						fprintf(stderr,"%d",c2>>j & 1);
					fprintf(stderr,"\n");
#endif
				}

				Mb_calcul_crc(trame,4+((longueur-1)/8),0);

				if (Mb_verbose)
				{
					fprintf(stderr,"answer :\n");
					for(i=0;i<6+((longueur-1)/8);i++)
						fprintf(stderr,"answer packet[%d] = %0x\n",i,trame[i]);
				}

				Mbs_write(trame,6+((longueur-1)/8));
				if(Mb_ptr_end_slve!=NULL)
					(*Mb_ptr_end_slve)(fonction,adresse,longueur);
				break;
				/*********************************************************************/
			case 0x03:
			case 0x04:
				/* read n byte */
				/* get adress */
				Mbs_read(&c1);
				Mbs_read(&c2);
				adresse=(c1<<8)+c2;
				trame[2]=c1;
				trame[3]=c2;
				if (Mb_verbose) fprintf(stderr,"adress %d (%x %x) \n",adresse,c1,c2);

				/* get length */
				Mbs_read(&c1);
				Mbs_read(&c2);
				longueur=(c1<<8)+c2;
				if (Mb_verbose) fprintf(stderr,"lenght %d \n",longueur);
				trame[4]=c1;
				trame[5]=c2;

				if( (adresse+longueur) > POSDIGMODBUS ){
					if (Mb_verbose) fprintf(stderr,"Peticion Posicion No Existente en Axis (Max: %d)... Finalizar\n",POSDIGMODBUS);
					if (Mb_verbose)
						for(i=0;i<=7;i++)
							fprintf(stderr,"sended packet[%d] = %0x\n",i,trame[i]);
					break;
				}

				/* get crc16 */
				Mbs_read(&c1);
				Mbs_read(&c2);
				trame[6]=c1;
				trame[7]=c2;
				if (Mb_verbose) fprintf(stderr,"crc  %x%x \n",c1,c2);

				/* check crc16 */
				if (Mb_test_crc(trame,6))
				{
					if (Mb_verbose) fprintf(stderr,"crc error\n");
					if (Mb_verbose)
						for(i=0;i<=7;i++)
							fprintf(stderr,"Error en crc packet[%d] = %0x\n",i,trame[i]);
					if(Mb_ptr_end_slve!=NULL)
						(*Mb_ptr_end_slve)(-1,0,0);
					break;
				}
				if (Mb_verbose)
					for(i=0;i<=7;i++)
						fprintf(stderr,"sended packet[%d] = %0x\n",i,trame[i]);

				/* comput answer packet */
				trame[2]=longueur*2;
				for (i=0;i<longueur;i++)
				{
					if (Mb_verbose)
						fprintf(stderr,"Mbs_data[%d] = %0x\n",adresse+i,Mbs_data[adresse+i]);
					trame[3+i*2]=Mbs_data[adresse+i]>>8;
					trame[4+i*2]=Mbs_data[adresse+i]&0xff;
				}

				Mb_calcul_crc(trame,(longueur*2)+3,0);

				if (Mb_verbose)
				{
					fprintf(stderr,"answer :\n");
					for(i=0;i<longueur*2+5;i++)
						fprintf(stderr,"answer packet[%d] = %0x\n",i,trame[i]);
				}

				Mbs_write(trame,(longueur*2)+5);
				if(Mb_ptr_end_slve!=NULL)
					(*Mb_ptr_end_slve)(fonction,adresse,longueur);

				break;
				/*********************************************************************/
			case 0x05:
				/* write on bit */
				/* get adress */
				Mbs_read(&c1);
				Mbs_read(&c2);
				adresse=(c1<<8)+c2;
				trame[2]=c1;
				trame[3]=c2;
				if (Mb_verbose) fprintf(stderr,"adress %d (%x %x) \n",adresse,c1,c2);

				/* get data */
				Mbs_read(&c1);
				Mbs_read(&c2);
				data=(c1<<8)+c2;
				if (Mb_verbose) fprintf(stderr,"data %d \n",data);
				trame[4]=c1;
				trame[5]=c2;
				if( (adresse) > (BBDDMODBUSLON-POSDIGMODBUS)*8 ){
					if (Mb_verbose) fprintf(stderr,"Peticion Posicion No Existente en Axis (Max: %d)... Finalizar\n",POSDIGMODBUS);
					if (Mb_verbose)
						for(i=0;i<=7;i++)
							fprintf(stderr,"sended packet[%d] = %0x\n",i,trame[i]);
					break;
				}
				/* get crc16 */
				Mbs_read(&c1);
				Mbs_read(&c2);
				trame[6]=c1;
				trame[7]=c2;
				if (Mb_verbose) fprintf(stderr,"crc  %x%x \n",c1,c2);
				/* check crc16 */
				if (Mb_test_crc(trame,6))
				{
					if (Mb_verbose) fprintf(stderr,"crc error\n");
					if(Mb_ptr_end_slve!=NULL)
						(*Mb_ptr_end_slve)(-1,0,0);
					break;
				}
				if (Mb_verbose)
					for(i=0;i<=7;i++)
						fprintf(stderr,"sended packet[%d] = %0x\n",i,trame[i]);

				// Se escribe bit correspondiente:
				if(trame[4]==0xff){
					Mbs_data[POSDIGMODBUS+(adresse/8)]|= 1 << (7-(adresse%8));
				}else{
					Mbs_data[POSDIGMODBUS+(adresse/8)]&= ~(1 << (7-(adresse%8)) );
				}

#ifdef DEBUG
				c1=Mbs_data[POSDIGMODBUS+(adresse/8)];
				fprintf(stderr,"auxch1 [%02X] Mbs_data[%d] [%02X]\n",c1,POSDIGMODBUS+(adresse/8),Mbs_data[POSDIGMODBUS+(adresse/8)]);
				for (i=7;i>=0;i--)
					fprintf(stderr,"%d",c1>>i & 1);
#endif

				/* answer trame is the same ;-)*/
				Mbs_write(trame,8);
				if(Mb_ptr_end_slve!=NULL)
					(*Mb_ptr_end_slve)(fonction,adresse,1);

				break;
				/*********************************************************************/
			case 0x06:
				/* write on byte */
				/* get adress */
				Mbs_read(&c1);
				Mbs_read(&c2);
				adresse=(c1<<8)+c2;
				trame[2]=c1;
				trame[3]=c2;
				if (Mb_verbose) fprintf(stderr,"adress %d (%x %x) \n",adresse,c1,c2);
				if( (adresse) > POSDIGMODBUS ){
					if (Mb_verbose) fprintf(stderr,"Peticion Posicion No Existente en Axis (Max: %d)... Finalizar\n",POSDIGMODBUS);
					break;
				}
				/* get data */
				Mbs_read(&c1);
				Mbs_read(&c2);
				data=(c1<<8)+c2;
				if (Mb_verbose) fprintf(stderr,"data %d \n",data);
				trame[4]=c1;
				trame[5]=c2;

				/* get crc16 */
				Mbs_read(&c1);
				Mbs_read(&c2);
				trame[6]=c1;
				trame[7]=c2;
				if (Mb_verbose) fprintf(stderr,"crc  %x%x \n",c1,c2);
				/* check crc16 */
				if (Mb_test_crc(trame,6))
				{
					if (Mb_verbose) fprintf(stderr,"crc error\n");
					if(Mb_ptr_end_slve!=NULL)
						(*Mb_ptr_end_slve)(-1,0,0);
					break;
				}
				if (Mb_verbose)
					for(i=0;i<=7;i++)
						fprintf(stderr,"sended packet[%d] = %0x\n",i,trame[i]);

				/* store data */
				Mbs_data[adresse]=data;
				if (Mb_verbose)
					fprintf(stderr,"data %d stored at : %d %x\n",data,adresse,adresse);

				/* answer trame is the same ;-)*/
				Mbs_write(trame,8);
				if(Mb_ptr_end_slve!=NULL)
					(*Mb_ptr_end_slve)(fonction,adresse,1);

				break;

				/*********************************************************************/
			case 0x07:
				/* read status */
				/* get crc16 */
				Mbs_read(&c1);
				Mbs_read(&c2);
				trame[2]=c1;
				trame[3]=c2;
				if (Mb_verbose) fprintf(stderr,"crc  %x%x \n",c1,c2);
				/* check crc16 */
				if (Mb_test_crc(trame,2))
				{
					if (Mb_verbose) fprintf(stderr,"crc error\n");
					if(Mb_ptr_end_slve!=NULL)
						(*Mb_ptr_end_slve)(-1,0,0);
					break;
				}
				if (Mb_verbose)
					for(i=0;i<=3;i++)
						fprintf(stderr,"sended packet[%d] = %0x\n",i,trame[i]);

				/* comput answer packet */
				trame[2]=Mb_status;

				Mb_calcul_crc(trame,3,0);

				if (Mb_verbose)
				{
					fprintf(stderr,"answer :\n");
					for(i=0;i<=4;i++)
						fprintf(stderr,"answer packet[%d] = %0x\n",i,trame[i]);
				}

				Mbs_write(trame,5);
				if(Mb_ptr_end_slve!=NULL)
					(*Mb_ptr_end_slve)(fonction,0,0);

				break;

				/*********************************************************************/
			case 0x08:
				/* line test */
				for (i=0;i<4;i++)
				{
					Mbs_read(&c);
					if (c!=0) break;
					trame[i+2]=c;
				}
				if (c!=0) break;

				/* get crc16 */
				Mbs_read(&c1);
				Mbs_read(&c2);
				trame[6]=c1;
				trame[7]=c2;
				if (Mb_verbose) fprintf(stderr,"crc  %x%x \n",c1,c2);
				/* check crc16 */
				if (Mb_test_crc(trame,6))
				{
					if (Mb_verbose) fprintf(stderr,"crc error\n");
					if(Mb_ptr_end_slve!=NULL)
						(*Mb_ptr_end_slve)(-1,0,0);
					break;
				}
				if (Mb_verbose)
					for(i=0;i<=7;i++)
						fprintf(stderr,"sended packet[%d] = %0x\n",i,trame[i]);

				/* return the same packet, it's cool */
				Mbs_write(trame,8);
				if(Mb_ptr_end_slve!=NULL)
					(*Mb_ptr_end_slve)(fonction,0,0);
				break;
				/*********************************************************************/
			case 0x0F:
				/* escribir n bit */
				/* get adress */
				Mbs_read(&c1);
				Mbs_read(&c2);
				adresse=(c1<<8)+c2;
				trame[2]=c1;
				trame[3]=c2;
				if (Mb_verbose) fprintf(stderr,"adress %d (%x %x) \n",adresse,c1,c2);

				/* get length */
				Mbs_read(&c1);
				Mbs_read(&c2);
				longueur=(c1<<8)+c2;
				if (Mb_verbose) fprintf(stderr,"lenght %d \n",longueur);
				trame[4]=c1;
				trame[5]=c2;

				/* read for nothing */
				Mbs_read(&c);
				trame[6]=c;

				/* read data to write */
				for (i=0;i<longueur/8;i++)
				{
					Mbs_read(&c1);
					Mbs_read(&c2);
					data_to_write[i]=(c1<<8)+c2;
					trame[7+(i*2)]=c1;
					trame[8+(i*2)]=c2;
				}

				/* get crc16 */
				Mbs_read(&c1);
				Mbs_read(&c2);
				trame[7+longueur/8]=c1;
				trame[8+longueur/8]=c2;
				if (Mb_verbose) fprintf(stderr,"crc  %x%x \n",c1,c2);

				/* check crc16 */
				if (Mb_test_crc(trame,7+longueur/8))
				{
					if (Mb_verbose) fprintf(stderr,"crc error\n");
					if(Mb_ptr_end_slve!=NULL)
						(*Mb_ptr_end_slve)(-1,0,0);
					break;
				}
				if (Mb_verbose)
					for(i=0;i<=8+longueur*2;i++)
						fprintf(stderr,"sended packet[%d] = %0x\n",i,trame[i]);


				if( (adresse+longueur) > (BBDDMODBUSLON-POSDIGMODBUS)*8 ){
					if (Mb_verbose) fprintf(stderr,"Peticion Posicion No Existente en Axis (Max: %d)... Finalizar\n",(BBDDMODBUSLON-POSDIGMODBUS)*8);
					break;
				}

				// Se escriben bit correspondientes:
				c1=Mbs_data[POSDIGMODBUS+(adresse/8)];

				c1=c1>>(8-(adresse%8));
				c1=c1<<(8-(adresse%8));

				//revertir orden byte recibido
				trame[7] = (trame[7] * 0x0202020202ULL & 0x010884422010ULL) % 1023;

				c2=trame[7]>>(adresse%8);

				Mbs_data[POSDIGMODBUS+(adresse/8)]=c1 | c2;
				c1=trame[7]<<(8-(adresse%8));

				for (i=1;i<1+((longueur-1)/8);i++){
					//revertir orden byte recibido
					trame[7+i] = (trame[7+i] * 0x0202020202ULL & 0x010884422010ULL) % 1023;
					c2=trame[7+i]>>(adresse%8);
					Mbs_data[POSDIGMODBUS+(adresse/8)+i]=c1 | c2;
					c1=trame[7+i]<<(8-(adresse%8));
				}
				Mbs_data[POSDIGMODBUS+(adresse/8)+i]=c1 | Mbs_data[POSDIGMODBUS+(adresse/8)+i];

				Mb_calcul_crc(trame,6,0);

				if (Mb_verbose)
				{
					fprintf(stderr,"answer :\n");
					for(i=0;i<6;i++)
						fprintf(stderr,"answer packet[%d] = %0x\n",i,trame[i]);
				}

				Mbs_write(trame,6);
				if(Mb_ptr_end_slve!=NULL)
					(*Mb_ptr_end_slve)(fonction,adresse,longueur);
				break;
				/*********************************************************************/
			case 0x10:
				/* write n byte  */
				/* get adress */
				Mbs_read(&c1);
				Mbs_read(&c2);
				adresse=(c1<<8)+c2;
				trame[2]=c1;
				trame[3]=c2;
				if (Mb_verbose) fprintf(stderr,"adress %d 0x%x%x \n",adresse,c1,c2);

				/* get length */
				Mbs_read(&c1);
				Mbs_read(&c2);
				longueur=(c1<<8)+c2;
				if (Mb_verbose) fprintf(stderr,"length %d \n",longueur);
				trame[4]=c1;
				trame[5]=c2;
				if( (adresse+longueur) > POSDIGMODBUS ){
					if (Mb_verbose) fprintf(stderr,"Peticion Posicion No Existente en Axis (Max: %d)... Finalizar\n",POSDIGMODBUS);
					break;
				}
				/* read for nothing */
				Mbs_read(&c);
				trame[6]=c;

				/* read data to write */
				for (i=0;i<longueur;i++)
				{
					Mbs_read(&c1);
					Mbs_read(&c2);
					data_to_write[i]=(c1<<8)+c2;
					trame[7+(i*2)]=c1;
					trame[8+(i*2)]=c2;
				}

				/* get crc16 */
				Mbs_read(&c1);
				Mbs_read(&c2);
				trame[7+longueur*2]=c1;
				trame[8+longueur*2]=c2;
				if (Mb_verbose) fprintf(stderr,"crc  %x%x \n",c1,c2);

				/* check crc16 */
				if (Mb_test_crc(trame,7+longueur*2))
				{
					if (Mb_verbose) fprintf(stderr,"crc error\n");
					if(Mb_ptr_end_slve!=NULL)
						(*Mb_ptr_end_slve)(-1,0,0);
					break;
				}
				if (Mb_verbose)
					for(i=0;i<=8+longueur*2;i++)
						fprintf(stderr,"sended packet[%d] = %0x\n",i,trame[i]);


				/* comput answer packet */
				Mb_calcul_crc(trame,6,0);

				if (Mb_verbose)
				{
					fprintf(stderr,"answer :\n");
					for(i=0;i<8;i++)
						fprintf(stderr,"answer packet[%d] = %0x\n",i,trame[i]);
				}

				Mbs_write(trame,8);

				/* copy data to modbus data*/
				for(i=0;i<longueur;i++)
				{
					Mbs_data[adresse+i]=data_to_write[i];
					if (Mb_verbose)
						fprintf(stderr,"data[%x] = %x\n",adresse+i,data_to_write[i]);
				}
				if(Mb_ptr_end_slve!=NULL)
					(*Mb_ptr_end_slve)(fonction,adresse,longueur);
				break;
				//      		default:
			}
			if (Mb_verbose)
				fprintf(stderr,"\n\nLiberar mutex rtu");
			pthread_mutex_unlock(&mutex);	//liberar cerrojo al responder peticion rtu
		}
	} while (1);
}


/***********************************************************************************
   	   Mb_slave_rtu : start slave thread
 ************************************************************************************
input :
-------
mbsdevice  : device to use
msslave    : slave number
ptrfoncsnd : function to call when slave send data (can be NULL if you don't whant to use it)
ptrfoncrcv : function to call when slave receive data (can be NULL if you don't whant to use it)
ptrfoncend : function to call when slave finish to send answer (can be NULL if you don't whant to use it)

no output
 ***********************************************************************************/
void Mb_slave_rtu(int mbsdevice,int mbsslave, void *ptrfoncsnd, void *ptrfoncrcv, void *ptrfoncend)
{
	Mbs_device=mbsdevice;
	Mbs_slave=mbsslave;
	Mb_ptr_snd_data=ptrfoncsnd;
	Mb_ptr_rcv_data=ptrfoncrcv;
	Mb_ptr_end_slve=ptrfoncend;
	pthread_create(&Mbs_thread_rtu, NULL,(void*)&Mbs_rtu,NULL);

}


/***********************************************************************************
   	   Mb_slave_rtu_stop : stop slave thread
 ************************************************************************************
no input
no output
 ***********************************************************************************/
void Mb_slave_rtu_stop(void)
{
	pthread_cancel(Mbs_thread_rtu);
}

void Mb_slave_init(void){
	pthread_mutex_init(&mutex, NULL);	// inicializar mutex (cerrojo)
}

void Mb_slave_stop(void){
	pthread_mutex_destroy(&mutex);	// finalizar mutex (cerrojo)
}

void Mb_slave_tcp(int socket, int esclavo){
	Mbs_socket=socket;
	Mbs_slave=esclavo;
	pthread_create(&Mbs_thread_tcp, NULL,(void*)&Mbs_tcp,NULL);

}

void Mb_slave_hora(){
	pthread_create(&Mbs_thread_hora, NULL,(void*)&Mbs_hora,NULL);
}

void Mb_slave_tcp_stop(void)
{
	pthread_cancel(Mbs_thread_tcp);
}

void Mb_slave_join_threads(void){
	pthread_join(Mbs_thread_tcp,NULL);
	pthread_join(Mbs_thread_rtu,NULL);
}

void Mb_slave_recibido_char(unsigned char *c){
	unsigned long time_actual=time(NULL);
	if(primer_char==1){
		primer_char=0;
		time_ultimo_char=time_actual;
		return;
	}
	if (Mb_verbose)
		fprintf(stderr,"\n\nRecibido char 0x%02X, primero(%hd) time_actual(%ld) time_ultimo(%ld) \n",c,primer_char,time_actual,time_ultimo_char);
	if( time_actual > (time_ultimo_char+TIMEOUT_MBS_RTU)){
		sprintf(aux,"%s\tProceso axismbus TIMEOUT_MBS_RTU Recibido char 0x%02X, primero(%hd) time_actual(%ld) time_ultimo(%ld)",c,primer_char,time_actual,time_ultimo_char);
		AxisLog(aux);
		pthread_mutex_unlock(&mutex);
	}
	time_ultimo_char=time_actual;
}

void Mb_slave_fin_envio(unsigned char *c){
	if (Mb_verbose)
		fprintf(stderr,"\n\nFin envio 0x%x",c);
	primer_char=1;
	pthread_mutex_unlock(&mutex);
}



