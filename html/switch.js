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
