#CREMONINI DAVIDE VR471360 UNIVR 2023

import firebase_admin
from firebase_admin import credentials
from firebase_admin import db
import plotly.graph_objs as go
import plotly.offline as pyo
from flask import Flask, render_template, request, redirect, url_for

#credenziali firebase
cred = credentials.Certificate('key.json')
firebase_admin.initialize_app(cred,{'databaseURL': 'private'})

#inizializzo app flask
app = Flask(__name__)

#funzione per leggere i dati dal db
def get_data():
    ref = db.reference('Utenti_time')
    data = ref.get()
    return data

#funzione per creare il grafico con i dati
def create_plot(data):
    fig = go.Figure()
    fig.add_trace(go.Bar(x=list(data.keys()), y=list(data.values()), name="Statistiche parcheggio"))
    fig.update_layout(title='Statistiche sull\'utilizzo', xaxis_title='', yaxis_title='secondi')
    return fig

#route per la pagina principale
@app.route('/')
def home():
    ref = db.reference('Richiesta_aiuto/Stato')
    status = ref.get()

    return render_template('index.html', status=status)

#route per modificare lo stato della richiesta
@app.route('/change_status', methods=['POST'])
def change_status():
    ref = db.reference('Richiesta_aiuto/Stato')
    ref.set(not ref.get()) 
    return redirect(url_for('home'))

#route per controllare lo stato della richiesta nel db, cosi posso far apparire la richiesta solo se true
@app.route('/check_status')
def check_status():
    ref = db.reference('Richiesta_aiuto/Stato')
    status = ref.get()
    return str(status)

#route per la pagina con il grafico
@app.route('/graph')
def graph():
    data = get_data()
    fig = create_plot(data)
    graph = pyo.plot(fig, output_type='div')
    return render_template('graph.html', graph=graph)


if __name__ == '__main__':
    app.run(debug=True)

