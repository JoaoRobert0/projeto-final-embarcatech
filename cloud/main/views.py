from django.shortcuts import render
from django.views import View
from .models import Registro

class RegistroView(View):
    def get(self, request):
        registros = Registro.objects.order_by('-id')

        visitante_qtd = registros.filter(cargo='Visitante').count()
        bolsista_qtd = registros.filter(cargo='Bolsista').count()
        total = visitante_qtd + bolsista_qtd


        return render(request, 'registro.html', {
            'registros': registros,
            'visitante_qtd': visitante_qtd,
            'bolsista_qtd': bolsista_qtd,
            'total': total
            })