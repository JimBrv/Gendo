<?php if (!defined('BASEPATH')) exit('No direct script access allowed');

class Server_mdl extends CI_Model
{
    	/* Table db */
	const TBL_OVPN_SERVER      = 'server';
    private $db_ovpn;
    
	/* default we load gendo openvpn database, not radius DB */
	public function __construct()
	{
		$this->db_ovpn   = $this->load->database('openvpn', TRUE);
	}
	
	public function get_all() 
	{
        $data = FALSE;

		$query = $this->db_ovpn->get(self::TBL_OVPN_SERVER);		
		$num   = $query->num_rows();
        $data = $query->result_array();
		$query->free_result();
		
		return $data;
	}
}
