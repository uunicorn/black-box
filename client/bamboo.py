import string
import sys
import time
import threading

import json
from urllib import urlencode
import httplib2 as http

import cfg

class Bamboo(threading.Thread):
    def __init__(self, plan):
        threading.Thread.__init__(self)
        self.setDaemon(True)
        self.plan = plan

        self.h = http.Http()

        self.headers = {
            'Accept': 'application/json',
        }

        self.h.add_credentials(cfg.username, cfg.password)

    def wget(self, url):
        try:
            rc, content = self.h.request(uri=url, headers=self.headers)

            if rc.status != 200:
                raise Exception("oops: %d" % rc.status)

#        print "----%s----\n%s\n" % (url, content)
            return content
        except:
            print url
            raise

    def trigger(self):
        data = dict(executeAllStages="true")
        rc, content = self.h.request(
                uri='http://bamboo:8085/rest/api/latest/queue/%s?os_authType=basic' % self.plan, 
                headers=self.headers, 
                method='POST', 
                body=urlencode(data))
        print "trigger: %d (%s)" % (rc.status, content)

    def update(self, leds, percent):
        print("%s: leds=%d percent=%03d" % (self.plan, leds, percent))

    def run(self):
        while True:
            try:
                self.query()
            except Exception as e:
                print e

            time.sleep(10)

    def query(self):
        url = "http://bamboo:8085/rest/api/latest/result/%s/latest?os_authType=basic&expand=plan" % self.plan
        build = json.loads(self.wget(url))

        if build['state'] == 'Successful':
            leds = 4
        else:
            leds = 1

        percent = 100

        if build['plan']['isBuilding']:
            leds |= 2

            url = "http://bamboo:8085/ajax/planStatusHistoryNeighbouringSummaries.action?planKey=%s&os_authType=basic" % self.plan
            summaries = json.loads(self.wget(url))["navigableSummaries"]
            active = filter(lambda x: x["buildStatus"] == "InProgress" and x["active"], summaries)
            active = map(lambda x: x["buildNumber"], active)

#            currentBuild = int(build["buildNumber"])+1
            done = 1.0
            for a in active:
                url = "http://bamboo:8085/rest/api/latest/result/status/%s-%d?os_authType=basic" % (self.plan, a)
                status = json.loads(self.wget(url))
                done = done * status["progress"]["percentageCompleted"]

            percent = int(done*100)

        self.update(leds, percent)

