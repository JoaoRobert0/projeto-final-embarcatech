from django.urls import path
from .views import *

urlpatterns = [
    path('', RegistroView.as_view()),
]