# ---
# jupyter:
#   jupytext:
#     formats: ipynb,py:light
#     text_representation:
#       extension: .py
#       format_name: light
#       format_version: '1.5'
#       jupytext_version: 1.11.3
#   kernelspec:
#     display_name: Python 3
#     language: python
#     name: python3
# ---

import pandas as pd
import numpy as np

metro_df = pd.read_csv("/scratch/zwang/tmp/redfin_metro_market_tracker.tsv000", delimiter='\t')

metro_df.describe()

# +
# metro_df.info()
# -

ny_df = metro_df[metro_df["region"] == 'New York, NY metro area'].sort_values(by='last_updated')

ny_df["period_begin"] = pd.to_datetime(ny_df["period_begin"])

ny_df = ny_df.sort_values(by="period_begin")
condo_df = ny_df[ny_df["property_type"] == "Condo/Co-op"]

# +
import plotly.express as px

fig = px.line(ny_df, x="period_begin", y="median_sale_price", title='New York Metro Area median sale price by property type', color="property_type")
fig.show()


# +
pd.set_option('display.max_colwidth', None)
pd.set_option('display.max_columns', None)

condo_df[condo_df["period_begin"] == "Jun 1, 2015"]

# +
import plotly.graph_objects as go
from plotly.subplots import make_subplots

# Create figure with secondary y-axis
fig = make_subplots(specs=[[{"secondary_y": True}]])

# Add traces
fig.add_trace(
    go.Scatter(x=condo_df["period_begin"], y=condo_df["median_ppsf"], name="median ppsf"),
    secondary_y=False,
)

fig.add_trace(
    go.Scatter(x=condo_df["period_begin"], y=condo_df["homes_sold"], name="number of homes sold"),
    secondary_y=True,
)

# Add figure title
fig.update_layout(
    title_text="New York Metro Area condo median ppsf and homes sold"
)

# Set x-axis title
fig.update_xaxes(title_text="time")

# Set y-axes titles
fig.update_yaxes(title_text="median ppsf", secondary_y=False)
fig.update_yaxes(title_text="number of homes sold", secondary_y=True)

fig.show()
# -

neighborhood_df = pd.read_csv("/scratch/zwang/tmp/neighborhood_market_tracker.tsv000", delimiter='\t')

nynj_df = neighborhood_df[(neighborhood_df['region'].str.contains("New York, NY")) | (neighborhood_df['region'].str.contains("Jersey City, NJ"))]

hudson_df = neighborhood_df[neighborhood_df['region'] == 'Jersey City, NJ - Hudson Exchange']

# +
pd.set_option('display.max_rows', None)

# nynj_df[["median_ppsf", "region"]].groupby('region').count().sort_values("median_ppsf")
nynj_df.head()
# -

zipcode_df = pd.read_csv("/scratch/zwang/tmp/zip_code_market_tracker.tsv000", delimiter='\t')
zipcode_df["zipcode"] = zipcode_df["region"].str[-5:]

zipcode_nynj_df = zipcode_df[(zipcode_df['parent_metro_region'].str.contains("New York, NY"))]
zipcode_allres_df = zipcode_nynj_df[(zipcode_nynj_df["property_type"] == "All Residential")]

zipcode_allres_df.head()

# +
import json
nycgeojson = {}
with open("/scratch/zwang/tmp/ny.geojson", "r") as rfile:
    nycgeojson = json.loads(rfile.read())

njgeojson = {}
with open("/scratch/zwang/tmp/nj.geojson", "r") as rfile:
    njgeojson = json.loads(rfile.read())
    for area in njgeojson["features"]:
        area["properties"]["postalCode"] = area["properties"]["GEOID10"]

nynj_geojson = nycgeojson
nynj_geojson["features"] += njgeojson["features"]
# -

latest_df = zipcode_allres_df[zipcode_allres_df['period_begin'] == '2022-01-01']

# +
fig = px.choropleth(
    latest_df,
    geojson=nynj_geojson,
    locations='zipcode',
    color='median_ppsf',
    color_continuous_scale="Reds",
    featureidkey='properties.postalCode',
    range_color=(500, 2500),
)
fig.update_geos(fitbounds="locations", visible=False)

fig.update_layout(margin={"r":0,"t":0,"l":0,"b":0}, title="NY / NJ median ppsf by zipcode 2022-01-01",)
fig.show()
# -





# +
import plotly
plotly.offline.init_notebook_mode()

sharenb()
# -


