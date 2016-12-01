<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');
    $this->load->view('header');
?>


<div class="div-user-center" >
    <div class="div-user-board">
        <span class="tb-list-title">用户信息</span>
        <table class="tb-list">
            <?php 
                echo "<tr class=\"tr-content\"> <td class=\"td-name\" >账号：</td>".
                "<td class=\"td-content\">".$this->session->userdata['username']."</td>".
                "<td class=\"td-memo\"> </td>".
                "</tr>";
            ?>
            <?php 
                echo "<tr class=\"tr-content\"> <td class=\"td-name\" >昵称：</td>".
                "<td class=\"td-content\">".$this->session->userdata['name']."</td>".
                "<td class=\"td-memo\"> <a href=\"user/change_nick\">"."修改"."</td>".
                "</tr>";
            ?>
            <?php 
                echo "<tr class=\"tr-content\"> <td class=\"td-name\" >注册时间：</td>".
                "<td class=\"td-content\">".$this->session->userdata['creation']."</td>".
                "<td class=\"td-memo\">"."</td>".
                "</tr>";
            ?>  
            <?php 
                echo "<tr class=\"tr-content\"> <td class=\"td-name\" >邮箱：</td>".
                "<td class=\"td-content\">".$this->session->userdata['email']."</td>".
                "<td class=\"td-memo\"> <a href=\"user/change_email\">"."修改"."</td>".
                "</tr>";
            ?>  
            <?php
                $info = "<tr class=\"tr-content\"> <td class=\"td-name\" >级别：</td>";
                
                
                if ($this->session->userdata['level'] != 0) {
                    $level = "<td class=\"td-content\">"."VIP用户"."</td>";
                    $info2 = "<td class=\"td-memo\"></td>"."</tr>";
                }else{
                    $level = "<td class=\"td-content\">"."免费用户"."</td>";    
                    $info2 = "<td class=\"td-memo\">".
                             "<a href=\"service\">"."升级为VIP，专享更多连线服务!".
                             "</td>".
                             "</tr>";
                }
                $msg = $info.$level.$info2;
                echo $msg;
            ?>

            <?php 
                $info = "<tr class=\"tr-content\"> <td class=\"td-name\" >使用偏好：</td>".
                "<td class=\"td-content\">";
               
                if ($this->session->userdata['hobby'] & 1) {
                    $level = "网页";
                }
                if ($this->session->userdata['hobby'] & 2) {
                    $level = "视频";
                }
                if ($this->session->userdata['hobby'] & 4) {
                    $level = "游戏";;
                }
                if ($this->session->userdata['hobby'] == 0) {
                    $level = "还没猜到";;
                }
                $msg = $info.$level."</td>";
                $msg = $msg."<td class=\"td-memo\"></td>"."</tr>";
                echo $msg;
            ?>


            <?php
                echo "<tr class=\"tr-content\"> <td class=\"td-name\" >订购服务：</td>".
                "<td class=\"td-content\">".$this->session->userdata['quota_name']."</td>".
                "<td class=\"td-memo\"></td>"."</tr>";
            ?> 
            <?php
                echo "<tr class=\"tr-content\"> <td class=\"td-name\" >开始时间：</td>".
                "<td class=\"td-content\">".$this->session->userdata['quota_start']."</td>".
                "<td class=\"td-memo\"></td>"."</tr>";
            ?> 
            <?php
                echo "<tr class=\"tr-content\"> <td class=\"td-name\" >结束时间：</td>".
                "<td class=\"td-content\">".$this->session->userdata['quota_expire']."</td>";
                $end = strtotime($this->session->userdata['quota_expire']);
                $now = time();
                $days = ceil(($end - $now)/(3600*24));
               
                if ($days <= 10) {
                    echo "<td class=\"td-memo\"> <a href=\"service\" style=\"color:red;\">"."还有".$days."天到期，请充值"."</td>";
                }else{
                    echo "<td class=\"td-memo\">"."到期时间还早！"."</td>";
                }
            ?>
            
            <?php
                $traf1 = $this->session->userdata['quota_used'];
                $traf2 = $this->session->userdata['quota_bytes'];
                echo "<tr class=\"tr-content\"> <td class=\"td-name\" >服务流量：</td>".
                "<td class=\"td-content\">".$traf1."(".ceil($traf1/(1024*1024))."M)".
                "/".$traf2."(".ceil($traf2/(1024*1024))."M)"."</td>";

                if ($traf1 >= ($traf2*0.7)) {
                    echo "<td class=\"td-memo\">"."流量不多了"."</td>"."</tr>";
                }else{
                    echo "<td class=\"td-memo\">"."流量充足"."</td>"."</tr>";
                }                
            ?>
            
            <?php
                $traf_all = $this->session->userdata['history_bytes'];
                echo "<tr class=\"tr-content\"> <td class=\"td-name\" >历史流量：</td>".
                "<td class=\"td-content\">".$traf_all."(".ceil($traf_all/(1024*1024))."M)"."</td>";
                if ($traf_all <= (1024*1024*1024)) {
                    echo "<td class=\"td-memo\">"."勋章：流量小乌龟，下一级：10G"."</td>"."</tr>";
                } elseif ($traf_all <= (1024*1024*1024*10)) {
                    echo "<td class=\"td-memo\">"."勋章：流量小白兔，下一级：50G"."</td>"."</tr>";
                } elseif ($traf_all <= (1024*1024*1024*50)) {
                    echo "<td class=\"td-memo\">"."勋章：流量大熊猫，下一级：100G"."</td>"."</tr>";
                } elseif ($traf_all <= (1024*1024*1024*100)) {
                    echo "<td class=\"td-memo\">"."勋章：流量巨无霸，下一级：500G"."</td>"."</tr>";
                } elseif ($traf_all <= (1024*1024*1024*500)) {
                    echo "<td class=\"td-memo\">"."勋章：流量天下无双，下一级：1T"."</td>"."</tr>";
                } else {
                    echo "<td class=\"td-memo\">"."勋章：流量天上天下无双，下一级：10T"."</td>"."</tr>";
                }
                "</tr>";
             ?>

             <?php
                $info1 = "<tr class=\"tr-content\">".
                         "<td class=\"td-name\" >当前状态：</td>";
                         
                
                if ($this->session->userdata['active'] == 0) {
                    $info2 = "<td class=\"td-content\">".
                             "<span style=\"color:red;\">超期冻结</span>".
                             "</td>";
                    $info3 = "<td class=\"td-memo\">".
                             "<a href=\"service\">"."sorry,服务已到期,请进行充值".
                             "</td>".
                             "</tr>";
                    $msg = $info1.$info2.$info3;
                }else{
                    $info2 = "<td class=\"td-content\">".
                             "正常可用".
                             "</td>";
                    $info3 = "<td class=\"td-memo\"></td>".
                             "</tr>";
                    $msg = $info1.$info2.$info3;
                }
                echo $msg;
             ?>
             <!--
             <?php
                echo "<tr class=\"tr-content\"> <td class=\"td-name\" >推荐人：</td>".
                "<td class=\"td-content\">".$this->session->userdata['refered']."</td>".
                "<td class=\"td-memo\"></td>"."</tr>";
             ?>  -->               
            </table>
        </div>
    
    <div class="div-svc-board">
    <span class="tb-list-title">服务订购记录</span>
    <p class="msg-normal" style="margin-left:30px;font-size:14px;">如果订购服务后清单没刷新，请主动退出再次登录，即可看到最新订购信息！ </p>
    <table class="tb-list">
          <tr class="tr-header">
              <td class="td-svc-No">序号</td>
              <td class="td-svc-serial">订单号</td>
              <td class="td-svc-name">服务类型</td>
              <td class="td-svc-cost">价格(元)</td>
              <td class="td-svc-ordertime">订购时间</td>
              <td class="td-svc-end">结束时间</td>
              <td class="td-svc-cost">状态</td>
          </tr>
          <?php
               $cnt = 0;
               foreach ($order as $key =>$row) {
                   $cnt++;
                   echo "<tr class=\"tr-content\"> <td class=\"td-svc-No\">".$cnt."</td>";
                   echo "<td class=\"td-svc-serial\" >".$row['serial']."</td>";
                   echo "<td class=\"td-svc-name\" >".$row['proname']."</td>";               
                   echo "<td class=\"td-svc-cost\" >".$row['proprice']."</td>";
                   echo "<td class=\"td-svc-ordertime\" >".$row['ordertime']."</td>";
                   echo "<td class=\"td-svc-end\" >".$row['proend']."</td>";
                   if ($row['state'] == 0) {
                       echo "<td class=\"td-svc-cost\">"."<a href=\"#\" style=\"color:#ff1493;font-size:12px;\">"."未付款"."</a>"."</td>";
                   }else{
                       echo "<td class=\"td-svc-cost\" style=\"color:green;font-size:12px;\">"."已付款"."</td>";
                   }
                   echo "</tr>";   
               }
          ?>                               
     </table>
    </div>
    
    <div class="div-svc-board">
    <span class="tb-list-title">用户推广奖励</span>
    <p class="msg-normal" style="margin-left:30px;font-size:14px;">您的推广链接：
        <?php 
            echo "<input type=\"text\" name=\"aff\" id=\"aff\" value=\"".
                 base_url()."register/referuser/".$this->session->userdata['id']."\""." style=\"padding-left:4px;font-size:12px;width:300px;\" readonly>";
        ?>
        <input type="button" value="点击复制链接" onclick="copyToClibBoard(document.getElementById('aff').value);">
    </p>
    <p class="msg-normal" style="margin-left:30px;font-size:14px;">您的推广人数：
        <?php 
            echo "<input type=\"text\" name=\"aff\" id=\"aff\" value=\"".
                 $this->session->userdata['bonus_cnt']."\""." style=\"padding-left:4px;font-size:12px;width:300px;color:#ff1493\" readonly>";
        ?>
    </p>
    <p class="msg-normal" style="margin-left:30px;font-size:14px;">您的可提佣金：
        <?php 
            echo "<input type=\"text\" name=\"aff\" id=\"aff\" value=\"".
                 $this->session->userdata['bonus']."\""." style=\"padding-left:4px;font-size:12px;width:300px;color:#ff1493;\" readonly>";
        ?>
    </p>
    
    <p class="msg-normal" style="margin-left:30px;font-size:12px;">
    1. 您可以将推广链接转发到博客、新浪微博、腾讯微博、各种论坛，或者作为您的QQ/微信/博客签名档；
    </p>
    <p class="msg-normal" style="margin-left:30px;font-size:12px;">   
    2. 用户通过您的推广链接进入跟斗云并成功注册；
    </p>
    <p class="msg-normal" style="margin-left:30px;font-size:12px;">
    3. 通过推广链接注册的用户成功订购服务，您将自动<span style="color:#ff1493"><strong>获得15%</strong></span>的佣金奖励；
    </p>
    <p class="msg-normal" style="margin-left:30px;font-size:12px;">
    4. 佣金<span style="color:#ff1493"><strong>满100元</strong></span>，即可申请支付宝提现；
    </p>
    </div>

</div>

<script type="text/javascript">
$(document).ready(function() {
    function copyToClibBoard(content){
        var clipBoardContent = ''; 
        clipBoardContent = content;
        window.clipboardData.setData("Text",clipBoardContent);
        alert('已复制到您的剪贴板');
    }
}
</script>

<?php $this->load->view('footer');?>