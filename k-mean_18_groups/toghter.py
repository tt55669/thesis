import numpy as np
import matplotlib.pyplot as plt

#colormap = np.array(['b', 'c', 'g','m','r','y','b', 'c', 'g','m','r','y','b', 'c', 'g','m','r','y']) 
colormap = np.array(['b', 'c', 'g','m','r','y','orangered', 'b', 'lime','purple','orange','dodgerblue','crimson', 'slategrey', 'peru','royalblue','pink','saddlebrown']) 
MTCD = 30000

t = np.random.random(size=MTCD) * 2 * np.pi - np.pi
x = np.cos(t)
y = np.sin(t)

i_set = np.arange(0,MTCD,1)

for i in i_set:
    distance = np.sqrt(np.random.random())
    x[i] = x[i] * distance*1000
    y[i] = y[i] * distance*1000

Head = 18
r = np.random.random(size=Head) * 2 * np.pi - np.pi
hx = np.cos(r)
hy = np.sin(r)

hi_set = np.arange(0,Head,1)

for i in hi_set:
    distance = np.sqrt(np.random.random())
    hx[i] = hx[i] * distance*1000
    hy[i] = hy[i] * distance*1000
# 兩點之間的距離
def dis(x, y, hx, hy):
    return int(((hx-x)**2 + (hy-y)**2)**0.5)

# 對每筆元素進行分群
def cluster(x, y, hx, hy):
    team = []
    for i in range(Head):
        team.append([])
    mid_dis = 99999999
    for i in range(MTCD):
        for j in range(Head):
            distant = dis(x[i], y[i], hx[j], hy[j])
            if distant < mid_dis:
                mid_dis = distant
                flag = j
        team[flag].append([x[i], y[i]])
        mid_dis = 99999999
    return team

# 對分群完的元素找出新的群集中心
def re_seed(team, hx, hy):
    sumx = 0
    sumy = 0
    new_seed = []
    for index, nodes in enumerate(team):
        if nodes == []:
            new_seed.append([hx[index], hy[index]])
        for node in nodes:
            sumx += node[0]
            sumy += node[1]
        if (len(nodes) != 0):
            new_seed.append([int(sumx/len(nodes)),int(sumy/len(nodes))])            
        sumx = 0
        sumy = 0
    nhx = []
    nhy = []
    for i in new_seed:
        nhx.append(i[0])
        nhy.append(i[1])

    return nhx, nhy

# k-means 分群
def kmeans(x, y, hx, hy, fig):
    team = cluster(x, y, hx, hy)
    nhx, nhy = re_seed(team, hx, hy)
    # 判斷群集中心是否不再更動
    if nhx == list(hx) and nhy == (hy):
        #print(team)
        #print(nhx,nhy)
        for index, nodes in enumerate(team):
            for i in range(len(nodes)):
                plt.scatter(nodes[i][0],nodes[i][1],c=colormap[index],s=15)
            plt.scatter(nhx[index],nhy[index],c='k',s=30,marker="^")
        plt.xlim(-1100,1100)
        plt.ylim(-1100,1100)
        plt.show()
        return 
    else:
        fig += 1
        kmeans(x, y, nhx, nhy, fig)

kmeans(x, y, hx, hy, fig=0)







