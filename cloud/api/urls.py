from django.urls import path
from .views import *

urlpatterns = [
    path('registro/', RegistroApi.as_view()),
]