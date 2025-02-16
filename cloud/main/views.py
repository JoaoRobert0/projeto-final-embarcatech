from django.shortcuts import render
from django.views import View
from .models import Registro

class RegistroView(View):
    def get(self, request):
        registros = Registro.objects.order_by('-id')
        return render(request, 'registro.html', {'registros': registros})