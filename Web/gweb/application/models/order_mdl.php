<?php if (!defined('BASEPATH')) exit('No direct script access allowed');

class Order_mdl extends CI_Model
{
    	/* Table db */
	const TBL_OVPN_ORDER      = 'order';
    private $db_ovpn;
    
	/* default we load gendo openvpn database, not radius DB */
	public function __construct()
	{
		$this->db_ovpn   = $this->load->database('openvpn', TRUE);
	}    
	
    public function get($uid, $username) 
	{
        $data = FALSE;
        
        $this->db_ovpn->where('uid', $uid);
		$query = $this->db_ovpn->get(self::TBL_OVPN_ORDER);
		$num   = $query->num_rows();
        $data  = $query->result_array();
		$query->free_result();		
		return $data;
	}
	
	public function get_by_serial($serial) 
	{
        $data = FALSE;
                
        $this->db_ovpn->where('serial', $serial);
		$query = $this->db_ovpn->get(self::TBL_OVPN_ORDER);
		$num   = $query->num_rows();
        $data  = $query->row_array();
		$query->free_result();		
		return $data;
	}
	
	public function add($serial, $uid, $username, $proid, $proname, $proprice, $prostart, $proend, $pay_platform='alipay', $pay_sn='0000')
	{	    	    
	    $order = array('serial'    => $serial,
	                   'ordertime' => date("Y-m-d H:i:s"),
	                   'uid'       => $uid,
	                   'username'  => $username,
	                   'proid'     => $proid,
	                   'proname'   => $proname,
	                   'proprice'  => $proprice,
	                   'prostart'  => $prostart,
	                   'proend'    => $proend,
	                   'platform'  => $pay_platform,
	                   'platform_serial' => $pay_sn);
	                   
	    $this->db_ovpn->insert(self::TBL_OVPN_ORDER,  $order);		
		if ($this->db_ovpn->affected_rows() !=1) {
		    log_message('error', "Add order=$username, $proid fail");
		    return -1;
		}
		return 0;
	}
	
	function is_exist($serial)
	{
        $this->db_ovpn->where('serial', $serial);
		$query = $this->db_ovpn->get(self::TBL_OVPN_ORDER);
		$num   = $query->num_rows();        
		$query->free_result();
		return $num == 1 ? TRUE : FALSE;   
	}
	
	function update($serial, $platform, $platform_serial, $status)
	{
        if ($this->is_exist($serial) == FALSE) {
            /* No order, Since it created before jumping to pay platform, it a fake update */
            log_message('error', "OrderUpdate [$serial, $status $platform] DB record not find!!!");
            return -1;            
        }
        
	 	$sql = "update `order` set state='$status', platform='$platform', platform_serial='$platform_serial' where serial='$serial'";
		              
		$this->db_ovpn->query($sql);
		if ($this->db_ovpn->affected_rows() != 1) {
		    log_message('error', "OrderUpdate [$serial, $status, $platform] failed!!!!");
		}else{
		    log_message('error', "OrderUpdate [$serial, $status, $platform] OK!");
		}        
		return 0;
	}
}