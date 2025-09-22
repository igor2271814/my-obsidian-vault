import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt

# Чтение данных
data = pd.read_csv('primer.csv', index_col='Year')

# Очистка столбца Value: замена запятых на точки и преобразование в числовой формат
data['Value'] = data['Value'].replace(',', '', regex=True) # Удаление всех запятых
data['Value'] = pd.to_numeric(data['Value'], errors='coerce') # Преобразование в числовой формат

# Проверка, что преобразование выполнено успешно
print(data['Value'].head())

# Создание сводной таблицы для тепловой карты
pivot_data = data.pivot_table(values='Value', index='Industry_name_NZSIOC', columns='Variable_name')

# Построение тепловой карты
sns.heatmap(pivot_data, annot=True, fmt="g", cmap="viridis")

plt.show()