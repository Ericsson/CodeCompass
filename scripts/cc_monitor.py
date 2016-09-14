#!/usr/bin/env python

import datetime
import smtplib
import sys
import time
import urllib2

from email.mime.text import MIMEText

MONITOR_EMAIL  = 'cc_monitor@codecompass.com'
SLEEP_DURATION = 1800 # half an hour
DAY = datetime.timedelta(days = 1)

class Server(object):
    """ Represents a running CodeCompass webserver"""

    def __init__(self, url):
        self._url = url

    def __str__(self):
        return self._url

    __repr__ = __str__

    def isAlive(self):
        try:
            urllib2.urlopen(self._url)
            return True
        except urllib2.HTTPError as ex:
            # this server might need authentication
            # at least the webserver responded
            return True
        except Exception as ex:
            return False


class Subscriber(object):
    """" Represents a subscribed user for monitoring """
    def __init__(self, email):
        self._email = email

    def __str__(self):
        return self._email

    __repr__ = __str__

    def sendMail(self, subject, content):
        msg = MIMEText(content)

        msg['Subject'] = subject
        msg['From']    = MONITOR_EMAIL
        msg['To']      = self._email

        s = smtplib.SMTP('localhost')
        s.sendmail(MONITOR_EMAIL, [self._email], msg.as_string())

        s.quit()

class Monitor(object):
    """ Monitors the webservers and sends notification if needed """

    def __init__(self, subscribers, servers):
        self._subscribers = subscribers
        self._servers = servers

        self._downSince = {}
        self._lastReportTime = datetime.datetime.min

    def delFromDict(self, dictionary, server):
        if dictionary.has_key(server):
            del dictionary[server]

    def collect(self, time):
        for server in self._servers:
            if server.isAlive():
                self.delFromDict(self._downSince, server)
            else:
                if not self._downSince.has_key(server):
                    self._downSince[server] = time

    def notifySubscribers(self, time):
        newDead = False

        for server, downSince in self._downSince.iteritems():
            if downSince == time:
                print "New Dead", server, downSince
                newDead = True

        now = datetime.datetime.now()
        if newDead or now - self._lastReportTime >= DAY:
            self.sendMails()

    def sendMails(self):
        self._lastReportTime = datetime.datetime.now()

        if not self._downSince:
            print 'Nothing to send...'
            return

        subject = "CodeCompass Monitor"
        content = "Hi! \n\n"

        content += "The following CodeCompass servers are unavailable:\n\n"

        for server, downSince in self._downSince.iteritems():
            content += str(server) + " " + str(downSince) + "\n"

        content += "\n\nHave a nice day!\n\nCodeCompass Monitor\n"

        print "sending emails..."

        print content

        for subscriber in self._subscribers:
            try:
                subscriber.sendMail(subject, content)
            except:
                print "Couldn't send email to", subscriber


    def run(self):
        print "CodeCompass Monitor started, monitoring servers", self._servers
        print "Notifying subscribers", self._subscribers
        while True:
            now = datetime.datetime.now()

            if now.hour > 5: # servers might be down because of nightly builds
                print "Testing servers..."
                self.collect(now)
                self.notifySubscribers(now)

            time.sleep(SLEEP_DURATION)


def parseConfig(configfile, subscribers, servers):
    Servers     = 1
    Subscribers = 2

    mode = 0
    for line in [line.strip() for line in open(configfile)]:
        if line == '[Servers]':
            mode = Servers
        elif line == '[Subscribers]':
            mode = Subscribers
        elif line:
            if mode == Servers:
                servers.append(Server(line))
            elif mode == Subscribers:
                subscribers.append(Subscriber(line))



if len(sys.argv) < 2:
    sys.exit('Usage: %s <config-file>' % sys.argv[0])

configfile = sys.argv[1]

subscribers = []
servers     = []

parseConfig(configfile, subscribers, servers)

if not subscribers:
    sys.exit("No subscribers to notify!")

if not servers:
    sys.exit("No servers to monitor!")

monitor = Monitor(subscribers, servers)

monitor.run()


