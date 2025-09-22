import pandas as pd
import joblib
from pathlib import Path
from sklearn.pipeline import Pipeline
from sklearn.compose import ColumnTransformer
from sklearn.preprocessing import StandardScaler, OneHotEncoder
from sklearn.impute import SimpleImputer
from xgboost import XGBRegressor
from fastapi import FastAPI, File, UploadFile, HTTPException
from io import BytesIO
from pyngrok import ngrok
import uvicorn
from fastapi.middleware.cors import CORSMiddleware

app = FastAPI()

# Добавьте эти middleware
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"],
    expose_headers=["ngrok-skip-browser-warning"],
)

def train_model():
    """Обучение и сохранение модели"""
    try:
        df = pd.read_csv('Laptop_price.csv')
        X = df.drop(columns=['Price'])
        y = df['Price']

        num_features = X.select_dtypes(include=['int64', 'float64']).columns
        cat_features = X.select_dtypes(include=['object']).columns

        preprocessor = ColumnTransformer([
            ('num', Pipeline([
                ('imputer', SimpleImputer(strategy='median')),
                ('scaler', StandardScaler())
            ]), num_features),
            ('cat', Pipeline([
                ('imputer', SimpleImputer(strategy='most_frequent')),
                ('encoder', OneHotEncoder(handle_unknown='ignore'))
            ]), cat_features)
        ])

        model = Pipeline([
            ('preprocessor', preprocessor),
            ('model', XGBRegressor(n_estimators=100, learning_rate=0.1, max_depth=5))
        ]).fit(X, y)

        joblib.dump(model, 'laptop_price_model.pkl')
        return model

    except Exception as e:
        raise HTTPException(500, f"Ошибка обучения модели: {str(e)}")

# Загрузка модели при старте
try:
    model = joblib.load('laptop_price_model.pkl')
except:
    model = train_model()

@app.post("/predict/")
async def predict(file: UploadFile = File(...)):
    if not file.filename.endswith('.csv'):
        raise HTTPException(400, "Только CSV файлы поддерживаются")

    content = await file.read()
    try:
        df = pd.read_csv(BytesIO(content))
        predictions = model.predict(df)
        return {"predictions": predictions.tolist()}
    except Exception as e:
        raise HTTPException(400, f"Ошибка обработки файла: {str(e)}")

@app.get("/")
async def root():
    return {
        "message": "API для предсказания цен на ноутбуки",
        "usage": "Отправьте POST запрос на /predict/ с CSV файлом"
    }

if __name__ == "__main__":
    if not Path('Laptop_price.csv').exists():
        raise FileNotFoundError("Основной файл данных не найден")

    ngrok_tunnel = ngrok.connect(8000)
    print(f"API доступно по: {ngrok_tunnel.public_url}")
    print(f"Swagger docs: {ngrok_tunnel.public_url}/docs")

    uvicorn.run(app, host="0.0.0.0", port=8000)



