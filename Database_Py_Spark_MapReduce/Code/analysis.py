#!/usr/bin/env python3

# May first need:
# In your VM: sudo apt-get install libgeos-dev (brew install on Mac)
# pip3 install https://github.com/matplotlib/basemap/archive/v1.1.0.tar.gz

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import pandas as pd
import datetime
import numpy as np

from mpl_toolkits.basemap import Basemap as Basemap
from matplotlib.colors import rgb2hex
from matplotlib.patches import Polygon

"""
IMPORTANT
This is EXAMPLE code.
There are a few things missing:
1) You may need to play with the colors in the US map.
2) This code assumes you are running in Jupyter Notebook or on your own system.
   If you are using the VM, you will instead need to play with writing the images
   to PNG files with decent margins and sizes.
3) The US map only has code for the Positive case. I leave the negative case to you.
4) Alaska and Hawaii got dropped off the map, but it's late, and I want you to have this
   code. So, if you can fix Hawaii and Alask, ExTrA CrEdIt. The source contains info
   about adding them back.
"""


"""
PLOT 1: SENTIMENT OVER TIME (TIME SERIES PLOT)
"""
# Assumes a file called time_data.csv that has columns
# date, Positive, Negative. Use absolute path.

ts = pd.read_csv("task10_2.csv")
# Remove erroneous row.
ts = ts[ts['date'] != '2018-12-31']

plt.figure(figsize=(12,5))
ts.date = pd.to_datetime(ts['date'], format='%Y-%m-%d')
ts.set_index(['date'],inplace=True)

ax = ts.plot(title="President Trump Sentiment on /r/politics Over Time",
        color=['green', 'red'],
       ylim=(0, 1.05))
ax.plot()
plt.savefig("plot1-sentiment-over-time.png")

"""
PLOT 2: SENTIMENT BY STATE (POSITIVE AND NEGATIVE SEPARATELY)
"""


task10_3= pd.read_csv("task10_3.csv")

"""
You also need to download the following files. Put them somewhere convenient:
https://github.com/matplotlib/basemap/blob/master/examples/st99_d00.shp
https://github.com/matplotlib/basemap/blob/master/examples/st99_d00.dbf
https://github.com/matplotlib/basemap/blob/master/examples/st99_d00.shx

IF YOU USE WGET (CONVERT TO CURL IF YOU USE THAT) TO DOWNLOAD THE ABOVE FILES, YOU NEED TO USE 
wget "https://github.com/matplotlib/basemap/blob/master/examples/st99_d00.shp?raw=true"
wget "https://github.com/matplotlib/basemap/blob/master/examples/st99_d00.dbf?raw=true"
wget "https://github.com/matplotlib/basemap/blob/master/examples/st99_d00.shx?raw=true"

The rename the files to get rid of the ?raw=true

"""

# Lambert Conformal map of lower 48 states.
m = Basemap(llcrnrlon=-119, llcrnrlat=22, urcrnrlon=-64, urcrnrlat=49,
        projection='lcc', lat_1=33, lat_2=45, lon_0=-95)
shp_info = m.readshapefile('st99_d00','states',drawbounds=True)  # No extension specified in path here.

# hawaii and alaska - (FROM https://github.com/matplotlib/basemap/blob/master/examples/fillstates.py)

m_other_states = Basemap(llcrnrlon=-190,llcrnrlat=20,urcrnrlon=-143,urcrnrlat=46,
            projection='merc',lat_ts=20)  # do not change these numbers
m_other_states.readshapefile('st99_d00','states',drawbounds=True)  # No extension specified in path here.
AREA_1 = 0.005  # exclude small Hawaiian islands that are smaller than AREA_1
AREA_2 = AREA_1 * 30.0  # exclude Alaskan islands that are smaller than AREA_2
AK_SCALE = 0.19  # scale down Alaska to show as a map inset
HI_OFFSET_X = -1900000  # X coordinate offset amount to move Hawaii "beneath" Texas
HI_OFFSET_Y = 250000    # similar to above: Y offset for Hawaii
AK_OFFSET_X = -250000   # X offset for Alaska (These four values are obtained
AK_OFFSET_Y = -750000   # via manual trial and error, thus changing them is not recommended.)

pos_data = dict(zip(task10_3.state, task10_3.pct_pos))
neg_data = dict(zip(task10_3.state, task10_3.pct_neg))

# choose a color for each state based on sentiment.
pos_colors = {}
neg_colors = {}
statenames = []
pos_cmap = plt.cm.Greens # use 'hot' colormap
neg_cmap = plt.cm.Reds # use 'hot' colormap

pos_vmin = 0.26; pos_vmax = 0.35 # set range.
neg_vmin = 0.39; neg_vmax = 0.47
for shapedict in m.states_info:
    statename = shapedict['NAME']
    # skip DC and Puerto Rico.
    if statename not in ['District of Columbia', 'Puerto Rico']:
        pos = pos_data[statename]
        pos_colors[statename] = pos_cmap(( pos - pos_vmin )/( pos_vmax - pos_vmin))[:3]
        neg = neg_data[statename]
        neg_colors[statename] = neg_cmap(( neg - neg_vmin )/( neg_vmax - neg_vmin))[:3]
    statenames.append(statename)
# cycle through state names, color each one.

# POSITIVE MAP
ax = plt.gca() # get current axes instance
for nshape, seg in enumerate(m.states):
    # skip Puerto Rico and DC
    if statenames[nshape] not in ['District of Columbia', 'Puerto Rico']:
        color = rgb2hex(pos_colors[statenames[nshape]]) 
        poly = Polygon(seg, facecolor=color, edgecolor=color)
        ax.add_patch(poly)

for nshape, shapedict in enumerate(m_other_states.states_info):  # plot Alaska and Hawaii as map insets
    if shapedict['NAME'] in ['Alaska', 'Hawaii']:
        seg = m_other_states.states[int(shapedict['SHAPENUM'] - 1)]
        if shapedict['NAME'] == 'Hawaii' and float(shapedict['AREA']) > AREA_1:
            seg = [(x + HI_OFFSET_X, y + HI_OFFSET_Y) for x, y in seg]
        elif shapedict['NAME'] == 'Alaska' and float(shapedict['AREA']) > AREA_2:
            seg = [(x*AK_SCALE + AK_OFFSET_X, y*AK_SCALE + AK_OFFSET_Y)\
                   for x, y in seg]
        color = rgb2hex(pos_colors[statenames[nshape]]) 
        poly = Polygon(seg, facecolor=color, edgecolor=color)
        ax.add_patch(poly)
plt.title('Positive Trump Sentiment Across the US')
plt.savefig("plot2-positive-map.png")


# Negative MAP
ax = plt.gca() # get current axes instance
for nshape, seg in enumerate(m.states):
    # skip Puerto Rico and DC
    if statenames[nshape] not in ['District of Columbia', 'Puerto Rico']:
        color = rgb2hex(neg_colors[statenames[nshape]]) 
        poly = Polygon(seg, facecolor=color, edgecolor=color)
        ax.add_patch(poly)
for nshape, shapedict in enumerate(m_other_states.states_info):  # plot Alaska and Hawaii as map insets
    if shapedict['NAME'] in ['Alaska', 'Hawaii']:
        seg = m_other_states.states[int(shapedict['SHAPENUM'] - 1)]
        if shapedict['NAME'] == 'Hawaii' and float(shapedict['AREA']) > AREA_1:
            seg = [(x + HI_OFFSET_X, y + HI_OFFSET_Y) for x, y in seg]
        elif shapedict['NAME'] == 'Alaska' and float(shapedict['AREA']) > AREA_2:
            seg = [(x*AK_SCALE + AK_OFFSET_X, y*AK_SCALE + AK_OFFSET_Y)\
                   for x, y in seg]
        color = rgb2hex(neg_colors[statenames[nshape]]) 
        poly = Polygon(seg, facecolor=color, edgecolor=color)
        ax.add_patch(poly)
plt.title('Negative Trump Sentiment Across the US')
plt.savefig("plot2-negative-map.png")

# SOURCE: https://stackoverflow.com/questions/39742305/how-to-use-basemap-python-to-plot-us-with-50-states
# (this misses Alaska and Hawaii. If you can get them to work, EXTRA CREDIT)

"""
PLOT 3: 
Create a third map of the United States that computes the difference: %Positive - %Negative.
"""
# Lambert Conformal map of lower 48 states.
m = Basemap(llcrnrlon=-119, llcrnrlat=22, urcrnrlon=-64, urcrnrlat=49,
        projection='lcc', lat_1=33, lat_2=45, lon_0=-95)
shp_info = m.readshapefile('st99_d00','states',drawbounds=True)  # No extension specified in path here.

# choose a color for each state based on sentiment.
all_colors = {}
statenames = []
pos_cmap = plt.cm.Greens # use 'hot' colormap
neg_cmap = plt.cm.Reds # use 'hot' colormap


vals = []
for shapedict in m.states_info:
    statename = shapedict['NAME']
    # skip DC and Puerto Rico.
    if statename not in ['District of Columbia', 'Puerto Rico']:
        val = pos_data[statename] - neg_data[statename]
        if val < 0:
            val = -1 * val
        vals.append(val)
vmax = max(vals) + 0.01
vmin = min(vals) - 0.01

for shapedict in m.states_info:
    statename = shapedict['NAME']
    # skip DC and Puerto Rico.
    if statename not in ['District of Columbia', 'Puerto Rico']:
        val = pos_data[statename] - neg_data[statename]
        if val > 0:
            all_colors[statename] = pos_cmap((val-vmin )/(vmax-vmin))[:3]
        elif val < 0:
            val = -1 * val
            all_colors[statename] = neg_cmap((val-vmin )/(vmax-vmin))[:3]
        else:
            all_colors[statename] = (1,1,1,) # white?
    statenames.append(statename)

# Difference MAP
ax = plt.gca() # get current axes instance
for nshape, seg in enumerate(m.states):
    # skip Puerto Rico and DC
    if statenames[nshape] not in ['District of Columbia', 'Puerto Rico']:
        color = rgb2hex(all_colors[statenames[nshape]]) 
        poly = Polygon(seg, facecolor=color, edgecolor=color)
        ax.add_patch(poly)
for nshape, shapedict in enumerate(m_other_states.states_info):  # plot Alaska and Hawaii as map insets
    if shapedict['NAME'] in ['Alaska', 'Hawaii']:
        seg = m_other_states.states[int(shapedict['SHAPENUM'] - 1)]
        if shapedict['NAME'] == 'Hawaii' and float(shapedict['AREA']) > AREA_1:
            seg = [(x + HI_OFFSET_X, y + HI_OFFSET_Y) for x, y in seg]
        elif shapedict['NAME'] == 'Alaska' and float(shapedict['AREA']) > AREA_2:
            seg = [(x*AK_SCALE + AK_OFFSET_X, y*AK_SCALE + AK_OFFSET_Y)\
                   for x, y in seg]
        color = rgb2hex(all_colors[statenames[nshape]]) 
        poly = Polygon(seg, facecolor=color, edgecolor=color)
        ax.add_patch(poly)
plt.title('Sentiment Difference map of Trump Accross US')
plt.savefig("plot3-difference-map.png")


"""
PART 4 SHOULD BE DONE IN SPARK

NOTE: part 4 is generated by reddit_model.py. 
It is task10_4_top10Neg_stories.csv and task10_4_top10Pos_stories.csv
"""


"""
PLOT 5A: SENTIMENT BY STORY SCORE
"""
# What is the purpose of this? It helps us determine if the story score
# should be a feature in the model. Remember that /r/politics is pretty
# biased.

# Assumes a CSV file called submission_score.csv with the following coluns
# submission_score, Positive, Negative

story = pd.read_csv("task10_4_submissions.csv")
plt.figure(figsize=(12,5))
fig = plt.figure()
ax1 = fig.add_subplot(111)

ax1.scatter(story['score'], story['pct_pos'], s=10, c='b', marker="s", label='Positive')
ax1.scatter(story['score'], story['pct_neg'], s=10, c='r', marker="o", label='Negative')
plt.legend(loc='lower right');

plt.xlabel('President Trump Sentiment by Submission Score')
plt.ylabel("Percent Sentiment")
plt.savefig("plot5-sentiment-by-submission-score.png")

"""
PLOT 5B: SENTIMENT BY COMMENT SCORE
"""
# What is the purpose of this? It helps us determine if the comment score
# should be a feature in the model. Remember that /r/politics is pretty
# biased.

# Assumes a CSV file called comment_score.csv with the following columns
# comment_score, Positive, Negative

story = pd.read_csv("task10_4_comments.csv")
plt.figure(figsize=(12,5))
fig = plt.figure()
ax1 = fig.add_subplot(111)

ax1.scatter(story['comm_score'], story['pct_pos'], s=10, c='b', marker="s", label='Positive')
ax1.scatter(story['comm_score'], story['pct_neg'], s=10, c='r', marker="o", label='Negative')
plt.legend(loc='lower right');

plt.xlabel('President Trump Sentiment by Comment Score')
plt.ylabel("Percent Sentiment")
plt.savefig("plot5-sentiment-by-comment-score.png")
