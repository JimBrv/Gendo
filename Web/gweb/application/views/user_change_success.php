<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');
    $this->load->view('header');
?>
  
<script language="JavaScript">
<?php
    echo "alert(\"修改成功\");";   
    echo "history.back();";   
?>
</script>

<?php $this->load->view('footer');?>