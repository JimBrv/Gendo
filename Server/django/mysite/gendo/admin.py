#-*-coding:utf-8 -*-
from django.contrib import admin
from mysite.gendo.models import *

class UserAdmin(admin.ModelAdmin):
    list_display = ('username', 'password', 'format_creation_time', 'email', \
                    'quota_name',   'quota_bytes', 'quota_used','quota_cycle', \
		    'format_expire_time', 'history_bytes', 'enabled', 'level')

    ordering = ('-creation', 'username')
    search_fields = ('username',)

class LogAdmin(admin.ModelAdmin):
    list_display = ('username', 'server_ip', 'format_start_time', 'format_end_time',  \
                    'protocol', 'trusted_ip', 'bytes_received', 'bytes_sent')
    ordering = ('-start_time',)
    search_fields = ('username',)

class OnlineAdmin(admin.ModelAdmin):
    list_display = ('username', 'format_start_time', 'trusted_ip', 'protocol', \
                    'server_ip')
    ordering = ('-start_time',)
    search_fields = ('username',)

class ServerAdmin(admin.ModelAdmin):
    list_display = ('ip', 'name', 'info', 'proto_show', 'ssl_port', \
                    'cur_user', 'max_user', 'state')
    ordering = ('cur_user',)
    search_fields = ('name',)

admin.site.register(User,   UserAdmin)
admin.site.register(Log,    LogAdmin)
admin.site.register(Online, OnlineAdmin)
admin.site.register(Server, ServerAdmin)
