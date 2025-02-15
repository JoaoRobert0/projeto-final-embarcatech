from django.db import models

# Create your models here.
class Registro(models.Model):
    CARGO_CHOICES = [
        ('visitante', 'Visitante'),
        ('bolsista', 'Bolsista'),
    ]
    cargo = models.CharField(max_length=100, choices=CARGO_CHOICES)
    data_hora = models.DateTimeField(auto_now_add=True)

    def __str__(self):
        return self.cargo + ' - ' + str(self.data_hora)