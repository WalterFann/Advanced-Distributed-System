import com.rabbitmq.client.Channel;
import com.rabbitmq.client.Connection;
import com.rabbitmq.client.ConnectionFactory;
import com.rabbitmq.client.DeliverCallback;

public class Recv {

    private final static String QUEUE_NAME_SEND = "send";
    private final static String QUEUE_NAME_RECV = "recv";

    public static void main(String[] argv) throws Exception {
        ConnectionFactory factory = new ConnectionFactory();
        factory.setHost("18.221.175.74");
        Connection connection = factory.newConnection();
        Channel channel_send = connection.createChannel();
	Channel channel_recv = connection.createChannel();

        channel_send.queueDeclare(QUEUE_NAME_SEND, false, false, false, null);
	channel_recv.queueDeclare(QUEUE_NAME_RECV, false, false, false, null);
        System.out.println(" [*] Waiting for messages. To exit press CTRL+C");

        DeliverCallback deliverCallback = (consumerTag, delivery) -> {
            String message = new String(delivery.getBody(), "UTF-8");
	    String ACK = "ACK!";
	    channel_recv.basicPublish("",QUEUE_NAME_RECV, null, ACK.getBytes("UTF-8"));
            System.out.println(" [x] Received '" + message + "'");
        };
        channel_send.basicConsume(QUEUE_NAME_SEND, true, deliverCallback, consumerTag -> { });
    }
}
