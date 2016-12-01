from django.db import models

# Create your models here.
# rop user model

class User(models.Model):
    username      = models.CharField(max_length=32)
    password      = models.CharField(max_length=32)
    active        = models.IntegerField()
    creation      = models.DateTimeField()
    name          = models.CharField(max_length=64)
    email         = models.CharField(max_length=32)
    note          = models.CharField(max_length=256)
    quota_cycle   = models.IntegerField()
    quota_bytes   = models.IntegerField()
    quota_used    = models.IntegerField()
    quota_name    = models.CharField(max_length=128)
    quota_expire  = models.DateTimeField()
    history_bytes = models.IntegerField()
    enabled       = models.IntegerField()
    level         = models.IntegerField()

    def format_date(self, obj):
        return obj.strftime('%Y-%m-%d %H:%M:%S')

    def format_creation_time(self):
        return self.format_date(self.creation)
    format_creation_time.short_description = 'Register'

    def format_expire_time(self):
        return self.format_date(self.quota_expire)
    format_expire_time.short_description = 'Quota Expire'

    def convert_flag_to_state(self, flag):
        cur_name = 'Invalid'
        if flag == 1:
            cur_name = 'Offline'            
        if flag == 3: 
            cur_name = 'Online'
        if flag == 5:
            cur_name = 'Need Fix'            
        if flag == 7:
            cur_name = 'Need Fix'
        if flag == 0:
            cur_name = 'Invalid'            
        return cur_name 

    def flag_to_state(self):
        return self.convert_flag_to_state(self.active)
    flag_to_state.short_description = 'User State'   

    def __str__(self):
        return self.name
    
    class Meta:
        db_table = 'user'


class Log(models.Model):
    username     = models.CharField(max_length=32)    
    start_time   = models.DateTimeField()
    end_time     = models.DateTimeField()
    trusted_ip   = models.CharField(max_length=32)
    trusted_port = models.IntegerField()
    protocol     = models.CharField(max_length=16)
    remote_ip    = models.CharField(max_length=32)    
    remote_netmask = models.CharField(max_length=32)    
    bytes_received = models.IntegerField()
    bytes_sent     = models.IntegerField()
    status         = models.IntegerField()
    server_ip      = models.CharField(max_length=32)    

    def format_date(self, obj):
        return obj.strftime('%Y-%m-%d %H:%M:%S')

    def format_start_time(self):
        return self.format_date(self.start_time)
    format_start_time.short_description = 'Start Time'

    def format_end_time(self):
        return self.format_date(self.end_time)
    format_end_time.short_description = 'End Time'


    def __str__(self):
        return self.name

    class Meta:
        db_table = 'log'


class Online(models.Model):
    username      = models.CharField(max_length=32)
    start_time    = models.DateTimeField()
    trusted_ip    = models.CharField(max_length=32)
    trusted_port  = models.IntegerField()
    protocol      = models.CharField(max_length=16)
    remote_ip     = models.CharField(max_length=32)
    remote_netmask= models.CharField(max_length=32)
    server_ip     = models.CharField(max_length=32)

    def format_date(self, obj):
        return obj.strftime('%Y-%m-%d %H:%M:%S')

    def format_start_time(self):
        return self.format_date(self.start_time)
    format_start_time.short_description = 'Start Time'
    
    def __str__(self):
        return self.name

    class Meta:
        pass
        db_table = 'online'


class Server(models.Model):
    ip       = models.CharField(max_length=32)
    name     = models.CharField(max_length=32)
    info     = models.CharField(max_length=32)
    protocol = models.IntegerField()
    latency  = models.IntegerField()
    cur_user = models.IntegerField()
    max_user = models.IntegerField()
    state    = models.IntegerField()
    ssl_port = models.CharField(max_length=128)

    def format_date(self, obj):
        return obj.strftime('%Y-%m-%d %H:%M:%S')

    def convert_proto(self, proto):
        cur_name = ''
        if proto & 1:
            cur_name += 'PPTP'            
        if proto & 2: 
            cur_name += '/L2TP'
        if proto & 4:
            cur_name += '/SSL-UDP'            
        if proto & 8:
            cur_name += '/SSL-TCP'
        if proto == 0:
            cur_name = 'Invalid'            
        return cur_name 

    def proto_show(self):
        return self.convert_proto(self.protocol)
    proto_show.short_description = 'Protocol' 
    
    def __str__(self):
        return self.name

    class Meta:
        pass
        db_table = 'server'

