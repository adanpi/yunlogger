$(document).ready(function() {

$('#fijarCabecera').bind('click', function(e) {

 e.preventDefault();

    $('.tablaSaica03').fixedHeaderTable({ width: '750', height: '450', themeClass: 'fancyTable', altClass: 'odd', footer: true, cloneHeadToFoot: false, fixedColumn: false });
	$('.tablaSaica03').fixedHeaderTable('show');

}); 

$('#quitarCabecera').bind('click', function(e) {

 e.preventDefault();
 
     $('.tablaSaica03').fixedHeaderTable('destroy');
     
});

});


function Switch(elemId)
{
elem = document.getElementById(elemId);

if (elem.style.display == "none")
	{
	elem.style.display = "";
	}
else
	{
	elem.style.display = "none";
	}
}

function Desplegar(elemId)
{
elem = document.getElementById(elemId);
var texto = elem.firstChild.nodeValue;

if (texto.charAt(1)=='+')
	{	
	elem.firstChild.nodeValue = texto.replace("+","-");
	}
else
	{	
	elem.firstChild.nodeValue = texto.replace("-","+");
	}
}

function SwitchCol(col1,col2)
{
fila=document.getElementById('tabla').getElementsByTagName('tr');

if (fila[0].getElementsByTagName('td')[col1].style.display == "none")
	{
	for(i=0;i<fila.length;i++) fila[i].getElementsByTagName('td')[col1].style.display = fila[i].getElementsByTagName('td')[col2].style.display = "";
	}
else
	{
	for(i=0;i<fila.length;i++) fila[i].getElementsByTagName('td')[col1].style.display = fila[i].getElementsByTagName('td')[col2].style.display = "none";
	}
}


function creaHTTPRequest()
{
if(window.XMLHttpRequest)  // Mozilla, Safari,...
   {
    http_request = new XMLHttpRequest();
    if(http_request.overrideMimeType) {http_request.overrideMimeType('text/xml');}
   }
else if (window.ActiveXObject)  // IE
   {
    try {http_request = new ActiveXObject("Msxml2.XMLHTTP");}
    catch(e) 
        {
         try {http_request = new ActiveXObject("Microsoft.XMLHTTP");}
         catch (e) {}
        }
    }
if(!http_request)
   {
   alert('Error :( No es posible crear una instancia XMLHTTP');
   return false;
   }
}


  function muestraContenidos()
     {
      creaHTTPRequest();
      
      http_request.onreadystatechange = muestraContenido;
      http_request.open('GET', '192.168.1.60/datos.xml',true);
      http_request.send(null);

      function muestraContenido()
        {
         if(http_request.readyState == 4)
            {
            if(http_request.status == 200)
              {
               procesaXML();
              }
            else {alert(http_request.status + " - " + http_request.statusText);}
            }
        }


      function procesaXML()
        {
         var xmlDoc = http_request.responseXML;

         var info = xmlDoc.getElementsByTagName("info")[0];
         var nombre = info.getElementsByTagName("nombre")[0].firstChild.nodeValue;

         var descripcion = info.getElementsByTagName('descripcion')[0].firstChild.nodeValue;
         var numA = info.getElementsByTagName('numA')[0].firstChild.nodeValue;
         var numG = info.getElementsByTagName('numG')[0].firstChild.nodeValue;
         var numR = info.getElementsByTagName('numR')[0].firstChild.nodeValue;
         var numC = info.getElementsByTagName('numC')[0].firstChild.nodeValue;
         var status = info.getElementsByTagName('status')[0].firstChild.nodeValue;
         var fecha = info.getElementsByTagName('fecha')[0].firstChild.nodeValue;
         var segjq = info.getElementsByTagName('segjq')[0].firstChild.nodeValue;
         var segjc = info.getElementsByTagName('segjc')[0].firstChild.nodeValue;

         document.getElementById('nombre').firstChild.nodeValue = 'NOMBRE: ' + nombre;
         document.getElementById('descripcion').firstChild.nodeValue = 'DESCRIPCION: ' + descripcion;
         document.getElementById('dimension').firstChild.nodeValue = 'NumAna: ' + numA + '  NumGray: ' + numG + '  NumRs: ' + numR + '  NumCont: ' + numC;
         document.getElementById('status').firstChild.nodeValue = 'Status QM: ' + status;
         document.getElementById('fecha').firstChild.nodeValue = 'Fecha Qm: ' + fecha;
         document.getElementById('segjq').firstChild.nodeValue = 'SegJul Qm: ' + segjq + '  SegJulPer: ' + segjq;
         document.getElementById('segjc').firstChild.nodeValue = 'SegJul Cm: [' + segjc + ']';


         var qm = xmlDoc.getElementsByTagName("qm")[0];
         var ana = qm.getElementsByTagName("an");
         var gra = qm.getElementsByTagName("gr");
         var rs2 = qm.getElementsByTagName("rs");
         var con = qm.getElementsByTagName("co");

         var tag = ' --- ';
         var des = ' --- ';
         var cue = ' --- ';
         var val = ' --- ';
         var uni = ' --- ';

         if(ana.length >0) var txt = ' <tr class="cabecera"><td width="120">Tag</td><td width="240">Descripcion</td><td width="50">Cuentas</td><td width="50">Valor</td><td width="50">Unidades</td></tr>';

         for(i=0;i<ana.length;i++)
            {
             tag = des = cue = val = uni = ' --- ';

             try {tag = ana[i].getElementsByTagName("tag")[0].firstChild.nodeValue;} catch(er){break;}
             try {des = ana[i].getElementsByTagName("des")[0].firstChild.nodeValue;} catch(er){break;}
             try {cue = ana[i].getElementsByTagName("cue")[0].firstChild.nodeValue;} catch(er){break;}
             try {val = ana[i].getElementsByTagName("val")[0].firstChild.nodeValue;} catch(er){break;}
             try {uni = ana[i].getElementsByTagName("uni")[0].firstChild.nodeValue;} catch(er){break;}
             
             txt = txt + '\n <tr class="dato">';
             txt = txt + '<td>' + tag + '</td>';
             txt = txt + '<td>' + des + '</td>';
             txt = txt + '<td>' + cue + '</td>';
             txt = txt + '<td class="alt">' + val + '</td>';
             txt = txt + '<td>' + uni + '</td></tr>';
            }
         document.getElementById("tablaAnalogicas").innerHTML = txt;

         if(gra.length >0) var txt = ' <tr class="cabecera"><td width="120">Tag</td><td width="240">Descripcion</td><td width="50">Cuentas</td><td width="50">Valor</td><td width="50">Unidades</td></tr>';

         for(i=0;i<gra.length;i++)
            {
             tag = des = cue = val = uni = ' --- ';

             try {tag = gra[i].getElementsByTagName("tag")[0].firstChild.nodeValue;} catch(er){break;}
             try {des = gra[i].getElementsByTagName("des")[0].firstChild.nodeValue;} catch(er){break;}
             try {cue = gra[i].getElementsByTagName("cue")[0].firstChild.nodeValue;} catch(er){break;}
             try {val = gra[i].getElementsByTagName("val")[0].firstChild.nodeValue;} catch(er){break;}
             try {uni = gra[i].getElementsByTagName("uni")[0].firstChild.nodeValue;} catch(er){break;}
             
             txt = txt + '\n <tr class="dato">';
             txt = txt + '<td>' + tag + '</td>';
             txt = txt + '<td>' + des + '</td>';
             txt = txt + '<td>' + cue + '</td>';
             txt = txt + '<td class="alt">' + val + '</td>';
             txt = txt + '<td>' + uni + '</td></tr>';
            }
         document.getElementById("tablaGrays").innerHTML = txt;

         if(rs2.length >0) var txt = ' <tr class="cabecera"><td width="120">Tag</td><td width="240">Descripcion</td><td width="50">Cuentas</td><td width="50">Valor</td><td width="50">Unidades</td></tr>';

         for(i=0;i<rs2.length;i++)
            {
             tag = des = cue = val = uni = ' --- ';

             try {tag = rs2[i].getElementsByTagName("tag")[0].firstChild.nodeValue;} catch(er){break;}
             try {des = rs2[i].getElementsByTagName("des")[0].firstChild.nodeValue;} catch(er){break;}
             try {cue = rs2[i].getElementsByTagName("cue")[0].firstChild.nodeValue;} catch(er){break;}
             try {val = rs2[i].getElementsByTagName("val")[0].firstChild.nodeValue;} catch(er){break;}
             try {uni = rs2[i].getElementsByTagName("uni")[0].firstChild.nodeValue;} catch(er){break;}
             
             txt = txt + '\n <tr class="dato">';
             txt = txt + '<td>' + tag + '</td>';
             txt = txt + '<td>' + des + '</td>';
             txt = txt + '<td>' + cue + '</td>';
             txt = txt + '<td class="alt">' + val + '</td>';
             txt = txt + '<td>' + uni + '</td></tr>';
            }
         document.getElementById("tablaRS232").innerHTML = txt;

         if(con.length >0) var txt = ' <tr class="cabecera"><td width="120">Tag</td><td width="240">Descripcion</td><td width="50">Cuentas</td><td width="50">Valor</td><td width="50">Unidades</td></tr>';

         for(i=0;i<con.length;i++)
            {
             tag = des = cue = val = uni = ' --- ';

             try {tag = con[i].getElementsByTagName("tag")[0].firstChild.nodeValue;} catch(er){break;}
             try {des = con[i].getElementsByTagName("des")[0].firstChild.nodeValue;} catch(er){break;}
             try {cue = con[i].getElementsByTagName("cue")[0].firstChild.nodeValue;} catch(er){break;}
             try {val = con[i].getElementsByTagName("val")[0].firstChild.nodeValue;} catch(er){break;}
             try {uni = con[i].getElementsByTagName("uni")[0].firstChild.nodeValue;} catch(er){break;}
             
             txt = txt + '\n <tr class="dato">';
             txt = txt + '<td>' + tag + '</td>';
             txt = txt + '<td>' + des + '</td>';
             txt = txt + '<td>' + cue + '</td>';
             txt = txt + '<td class="alt">' + val + '</td>';
             txt = txt + '<td>' + uni + '</td></tr>';
            }
         document.getElementById("tablaContadores").innerHTML = txt;

        }
      
     }



function muestraContenidos2()
{
      creaHTTPRequest();
      
      http_request.onreadystatechange = muestraContenido;
      http_request.open('GET', 'AX00/usr-cgi/consultorioXML.sh',true);
//      http_request.open('GET', 'AX00/usr-cgi/datos2.xml',true);
//      http_request.open('GET', 'AX00/datos2.xml',true);
      http_request.send(null);

      function muestraContenido()
        {
         if(http_request.readyState == 4)
            {
            if(http_request.status == 200)
              {
               procesaXML2();
              }
            else {alert(http_request.status + " - " + http_request.statusText);}
            }
        }


        function procesaXML2()
        {
         var xmlDoc = http_request.responseXML;

         var info = xmlDoc.getElementsByTagName("info")[0];

         var nombre = info.getElementsByTagName("nombre")[0].firstChild.nodeValue;
         var descripcionR = info.getElementsByTagName('descRem')[0].firstChild.nodeValue;
         var descripcionS = info.getElementsByTagName('descSen')[0].firstChild.nodeValue;
         var inicio = parseInt(info.getElementsByTagName('ini')[0].firstChild.nodeValue); // inicio del primer quinceminutal, que usaremos como referencia

         document.getElementById('nombre').firstChild.nodeValue = 'NOMBRE: ' + nombre;
         document.getElementById('descripcionR').firstChild.nodeValue = 'DESCRIPCION REMOTA: ' + descripcionR;
         document.getElementById('descripcionS').firstChild.nodeValue = 'DESCRIPCION SENAL: ' + descripcionS;


         var qm = xmlDoc.getElementsByTagName("q");
         var fec = ' --- ';
         var sjq = 0;
         var sjp = 0;
         var sta = 0;
         var val = 0;

         if(qm.length >0) var txt = ' <tr class="cabecera"><td>Fecha</td><td>SegJulQm</td><td>SegJulPer</td><td>StatusQm</td><td>ValorQm</td></tr>';
         for(i=0;i<qm.length;i++)
            {
             fec = ' --- ';
             sjq = sjp = sta = val = 0;

             try {sjq = qm[i].getElementsByTagName("s")[0].firstChild.nodeValue;} catch(er){break;}
             try {sjp = qm[i].getElementsByTagName("sp")[0].firstChild.nodeValue;} catch(er){sjp=inicio + 900*sjq;} // Si no pone nada, es que sjp=sjq
             try {sta = qm[i].getElementsByTagName("t")[0].firstChild.nodeValue;} catch(er){break;}
             try {val = qm[i].getElementsByTagName("v")[0].firstChild.nodeValue;} catch(er){break;}
             
             txt = txt + '\n <tr class="dato">';
             txt = txt + '<td>' + segundoAFecha(inicio + 900*sjq) + '</td>';
             txt = txt + '<td>' + (inicio + 900*sjq) + '</td>';
             txt = txt + '<td>' + sjp + '</td>';
             txt = txt + '<td class="alt">' + sta + '</td>';
             txt = txt + '<td>' + val + '</td></tr>';
            }
         document.getElementById("tablaQM").innerHTML = txt;

        } 
}


function cargaRemota(nombre)
{
document.getElementById("titulo").innerHTML = nombre;

creaHTTPRequest();
      
http_request.onreadystatechange = rellenaContenido;
http_request.open('GET', 'portada.txt',true);
http_request.send(null);
}


function rellenaContenido()
{
         if(http_request.readyState == 4)
            {
            if(http_request.status == 200)
              {
               document.getElementById("contenido").innerHTML = http_request.responseText;
              }
            else {alert(http_request.status + " - " + http_request.statusText);}
            }
}

function segundoAFecha(segundos)
{
cuando = new Date();
cuando.setTime(segundos * 1000);
return (cuando.getDate() + '/' + (1+cuando.getMonth()) + '/' + cuando.getUTCFullYear() + ' ' + cuando.getUTCHours() + ':' + cuando.getUTCMinutes());
}

