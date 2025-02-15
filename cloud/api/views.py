from django.shortcuts import render
from django.views import View

import json
from django.http import JsonResponse
from main.models import Registro

class RegistroApi(View):
    def get(self, request):
        return JsonResponse({
            'mensagem': 'Para criar um registro, envie uma requisição POST, em .../api/registro, com um JSON contendo o campo "cargo" com o valor "Visitante" ou "Bolsista".'
        })
    
    def post(self, request):
        try:
            # Obtém o JSON enviado na requisição
            data = json.loads(request.body.decode('utf-8'))  
            
            # Pega o atributo 'cargo'
            cargo = data.get('cargo')

            # Verifica se 'cargo' foi enviado
            if not cargo:
                return JsonResponse({'erro': 'O campo cargo é obrigatório!'}, status=400)
            
            if cargo != 'Visitante' and cargo != 'Bolsista':
                return JsonResponse({'erro': 'O campo cargo deve ser "Visitante" ou "Bolsista"!'}, status=400)
            
            # Cria um novo registro
            Registro.objects.create(cargo=cargo)

            # Retorna uma mensagem de sucesso em JSON
            return JsonResponse({'mensagem': f'Cargo {cargo} registrado com sucesso!'}, status=201)
        
        except json.JSONDecodeError:
            return JsonResponse({'erro': 'Formato de JSON inválido!'}, status=400)
